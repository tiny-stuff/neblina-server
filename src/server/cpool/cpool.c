#include "cpool.h"

#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>

#include "util/error.h"

#include "../server.h"
#include "os/os.h"
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

    DBG("Creating thread %zu", ctx->thread_n);

    while (ctx->cpool->thread_running[ctx->thread_n]) {
        pthread_mutex_lock(&ctx->mutex);
        while (!ctx->should_wake)
            pthread_cond_wait(&ctx->cond, &ctx->mutex);
        ctx->should_wake = false;
        pthread_mutex_unlock(&ctx->mutex);

        DBG("Work from thread %zu!", ctx->thread_n);  // TODO
    }

    pthread_mutex_lock(&ctx->cpool->connection_threads_mutex);
    // ct->ready = true;
    pthread_mutex_unlock(&ctx->cpool->connection_threads_mutex);

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
            if (pthread_create(&cpool->threads[i], NULL, thread_function, &cpool->ctx[i]))
                FATAL_NON_RECOVERABLE("Unable to create thread.");
        }
    }

    return cpool;
}

void cpool_destroy(CPool* cpool)
{
    if (cpool->n_threads != SINGLE_THREADED) {
        pthread_mutex_destroy(&cpool->connection_threads_mutex);

        // end threads
        for (size_t i = 0; i < cpool->n_threads; ++i) {
            pthread_cond_signal(&cpool->ctx[i].cond);
            cpool->thread_running[i] = false;
        }
        for (size_t i = 0; i < cpool->n_threads; ++i) {
            pthread_join(cpool->threads[i], NULL);
            pthread_mutex_destroy(&cpool->ctx[i].mutex);
            pthread_cond_destroy(&cpool->ctx[i].cond);
            DBG("Thread %zu finalized", i);
        }

        // cleanup
        free(cpool->threads);
        free(cpool->thread_running);
    }
    free(cpool->ctx);

    free(cpool);
}

size_t cpool_least_populated_thread(CPool* cpool)
{
    size_t smallest_thread_n = 0, smallest_sz = SIZE_MAX;
    for (size_t i = 0; i < cpool->n_threads; ++i) {
        size_t count = 0;
        for (ConnectionThread* conn_th = cpool->connection_thread_map; conn_th != NULL; conn_th = conn_th->hh.next) {
            if (conn_th->thread_n == i)
                ++count;
        }
        if (count < smallest_sz)
            smallest_thread_n = i;
    }

    return smallest_thread_n;
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
        HASH_FIND_PTR(cpool->connection_thread_map, connection, ct);
        if (ct) {
            DBG("Connection removed from thread %zu", ct->thread_n);
            HASH_DEL(cpool->connection_thread_map, ct);
            free(ct);
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