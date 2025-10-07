#include "cpool.h"

#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>

#include "util/error.h"

#include "../server.h"
#include "os.h"
#include "uthash/uthash.h"

typedef struct ThreadContext {
    struct CPool*   cpool;
    size_t          thread_n;
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
    bool            should_wake;
} ThreadContext;

typedef struct ConnectionThread {
    Connection*    connection;
    size_t         thread_n;
    bool           ready;
    UT_hash_handle hh;
} ConnectionThread;

typedef struct CPool {
    Server*           server;
    size_t            n_threads;
    pthread_t*        threads;
    bool*             thread_running;
    ThreadContext*    ctx;
    ConnectionThread* connection_thread_map;
    pthread_mutex_t   connection_threads_mutex;
} CPool;


static void* thread_function(void* arg)
{
    ThreadContext* ctx = arg;

    while (ctx->cpool->thread_running[ctx->thread_n]) {

        // wait until work is available
        pthread_mutex_lock(&ctx->mutex);
        while (!ctx->should_wake)
            pthread_cond_wait(&ctx->cond, &ctx->mutex);
        ctx->should_wake = false;
        pthread_mutex_unlock(&ctx->mutex);

        // make a list of connections that are ready for work
        Connection** connections = NULL;
        size_t connections_sz = 0;
        pthread_mutex_lock(&ctx->cpool->connection_threads_mutex);
        for (ConnectionThread* conn_th = ctx->cpool->connection_thread_map; conn_th != NULL; conn_th = conn_th->hh.next) {
            if (conn_th->ready && conn_th->thread_n == ctx->thread_n) {
                ++connections_sz;
                connections = realloc(connections, connections_sz * sizeof(Connection *));
                connections[connections_sz - 1] = conn_th->connection;
                conn_th->ready = false;
            }
        }
        pthread_mutex_unlock(&ctx->cpool->connection_threads_mutex);

        // do the work
        for (size_t i = 0; i < connections_sz; ++i)
            server_flush_connection(ctx->cpool->server, connections[i]);

        free(connections);
    }

    return NULL;
}

CPool* cpool_create(size_t n_threads, Server* server)
{
    CPool* cpool = calloc(1, sizeof(CPool));
    cpool->server = server;

    // create threads
    cpool->n_threads = n_threads;
    if (n_threads != SINGLE_THREADED) {

        cpool->thread_running = calloc(n_threads, sizeof cpool->thread_running[0]);
        cpool->threads = calloc(n_threads, sizeof cpool->threads[0]);
        cpool->ctx = calloc(n_threads, sizeof cpool->ctx[0]);
        cpool->connection_thread_map = NULL;
        pthread_mutex_init(&cpool->connection_threads_mutex, NULL);

        for (size_t i = 0; i < n_threads; ++i) {
            cpool->thread_running[i] = true;
            cpool->ctx[i] = (ThreadContext) { .cpool = cpool, .thread_n = i, .should_wake = false };
            pthread_mutex_init(&cpool->ctx[i].mutex, NULL);
            pthread_cond_init(&cpool->ctx[i].cond, NULL);
            DBG("Creating thread %zu", i);
            if (pthread_create(&cpool->threads[i], NULL, thread_function, &cpool->ctx[i]))
                FATAL_NON_RECOVERABLE("Unable to create thread.");
        }
    }

    return cpool;
}

void cpool_destroy(CPool* cpool)
{
    if (cpool->n_threads != SINGLE_THREADED) {

        // end threads
        pthread_mutex_lock(&cpool->connection_threads_mutex);
        for (size_t i = 0; i < cpool->n_threads; ++i) {
            pthread_mutex_lock(&cpool->ctx[i].mutex);
            cpool->thread_running[i] = false;
            cpool->ctx[i].should_wake = true;
            pthread_mutex_unlock(&cpool->ctx[i].mutex);
        }
        pthread_mutex_unlock(&cpool->connection_threads_mutex);

        for (size_t i = 0; i < cpool->n_threads; ++i) {
            pthread_mutex_lock(&cpool->ctx[i].mutex);
            pthread_cond_signal(&cpool->ctx[i].cond);
            pthread_mutex_unlock(&cpool->ctx[i].mutex);
            pthread_join(cpool->threads[i], NULL);
            pthread_mutex_destroy(&cpool->ctx[i].mutex);
            pthread_cond_destroy(&cpool->ctx[i].cond);
            DBG("Thread %zu finalized", i);
        }

        pthread_mutex_destroy(&cpool->connection_threads_mutex);

        // cleanup
        free(cpool->threads);
        free(cpool->thread_running);
    }
    free(cpool->ctx);

    free(cpool);
}

size_t cpool_least_populated_thread(CPool* cpool)
{
    size_t* count_per_thread = calloc(cpool->n_threads, sizeof(size_t));

    for (ConnectionThread* conn_th = cpool->connection_thread_map; conn_th != NULL; conn_th = conn_th->hh.next)
        ++count_per_thread[conn_th->thread_n];

    size_t min_sz = SIZE_MAX, min_thread = 0;
    for (size_t i = 0; i < cpool->n_threads; ++i) {
        if (count_per_thread[i] < min_sz) {
            min_thread = i;
            min_sz = count_per_thread[i];
        }
    }

    free(count_per_thread);

    return min_thread;
}

void cpool_add_connection(CPool* cpool, Connection* connection)
{
    (void) connection;

    if (cpool->n_threads != SINGLE_THREADED) {
        // find least populated thread
        size_t thread_n = cpool_least_populated_thread(cpool);

        // add connection to thread
        ConnectionThread* ct = malloc(sizeof *ct);
        ct->thread_n = thread_n;
        ct->connection = connection;
        ct->ready = false;
        pthread_mutex_lock(&cpool->connection_threads_mutex);
        HASH_ADD_PTR(cpool->connection_thread_map, connection, ct);
        pthread_mutex_unlock(&cpool->connection_threads_mutex);

        DBG("New connection added to thread %zu", thread_n);
    }
}

void cpool_remove_connection(CPool* cpool, Connection* connection)
{
    (void) connection;

    if (cpool->n_threads != SINGLE_THREADED) {
        pthread_mutex_lock(&cpool->connection_threads_mutex);
        ConnectionThread* ct;
        HASH_FIND_PTR(cpool->connection_thread_map, &connection, ct);
        if (ct) {
            size_t thread_n = ct->thread_n;
            pthread_mutex_lock(&cpool->ctx[thread_n].mutex);
            DBG("Connection removed from thread %zu", ct->thread_n);
            HASH_DEL(cpool->connection_thread_map, ct);
            free(ct);
            pthread_mutex_unlock(&cpool->ctx[thread_n].mutex);
        }
        pthread_mutex_unlock(&cpool->connection_threads_mutex);
    }
}

void cpool_flush_connection(CPool* cpool, Connection* connection)
{
    if (cpool->n_threads != SINGLE_THREADED) {
        ConnectionThread* ct;
        HASH_FIND_PTR(cpool->connection_thread_map, &connection, ct);
        if (ct) {
            pthread_mutex_lock(&cpool->connection_threads_mutex);
            ct->ready = true;
            pthread_mutex_unlock(&cpool->connection_threads_mutex);

            pthread_mutex_lock(&cpool->ctx[ct->thread_n].mutex);
            cpool->ctx[ct->thread_n].should_wake = true;
            pthread_cond_signal(&cpool->ctx[ct->thread_n].cond);
            pthread_mutex_unlock(&cpool->ctx[ct->thread_n].mutex);
        }
    } else {
        server_flush_connection(cpool->server, connection);
    }
}