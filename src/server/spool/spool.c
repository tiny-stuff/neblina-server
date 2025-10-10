#include "spool.h"

#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <uthash/uthash.h>

#include "util/error.h"

#include "../server.h"
#include "os.h"
#include "util/alloc.h"

typedef struct ThreadContext {
    struct SessionPool*   cpool;
    size_t          thread_n;
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
    bool            should_wake;
} ThreadContext;

typedef struct SessionThread {
    Session*       session;
    size_t         thread_n;
    bool           ready;
    UT_hash_handle hh;
} SessionThread;

typedef struct SessionPool {
    Server*           server;
    size_t            n_threads;
    pthread_t*        threads;
    bool*             thread_running;
    ThreadContext*    ctx;
    SessionThread*    session_thread_map;
    pthread_mutex_t   session_threads_mutex;
} SessionPool;


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

        // make a list of sessions that are ready for work
        Session** sessions = NULL;
        size_t sessions_sz = 0;
        pthread_mutex_lock(&ctx->cpool->session_threads_mutex);
        for (SessionThread* session_th = ctx->cpool->session_thread_map; session_th != NULL; session_th = session_th->hh.next) {
            if (session_th->ready && session_th->thread_n == ctx->thread_n) {
                ++sessions_sz;
                sessions = REALLOC(sessions, sessions_sz * sizeof(Session *));
                sessions[sessions_sz - 1] = session_th->session;
                session_th->ready = false;
            }
        }
        pthread_mutex_unlock(&ctx->cpool->session_threads_mutex);

        // do the work
        for (size_t i = 0; i < sessions_sz; ++i)
            server_process_session(ctx->cpool->server, sessions[i]);

        free(sessions);
    }

    return NULL;
}

SessionPool* spool_create(size_t n_threads, Server* server)
{
    SessionPool* cpool = CALLOC(1, sizeof(SessionPool));
    cpool->server = server;

    // create threads
    cpool->n_threads = n_threads;
    if (n_threads != SINGLE_THREADED) {

        cpool->thread_running = CALLOC(n_threads, sizeof cpool->thread_running[0]);
        cpool->threads = CALLOC(n_threads, sizeof cpool->threads[0]);
        cpool->ctx = CALLOC(n_threads, sizeof cpool->ctx[0]);
        cpool->session_thread_map = NULL;
        pthread_mutex_init(&cpool->session_threads_mutex, NULL);

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

void spool_destroy(SessionPool* cpool)
{
    if (cpool->n_threads != SINGLE_THREADED) {

        // end threads
        pthread_mutex_lock(&cpool->session_threads_mutex);
        for (size_t i = 0; i < cpool->n_threads; ++i) {
            pthread_mutex_lock(&cpool->ctx[i].mutex);
            cpool->thread_running[i] = false;
            cpool->ctx[i].should_wake = true;
            pthread_mutex_unlock(&cpool->ctx[i].mutex);
        }
        pthread_mutex_unlock(&cpool->session_threads_mutex);

        for (size_t i = 0; i < cpool->n_threads; ++i) {
            pthread_mutex_lock(&cpool->ctx[i].mutex);
            pthread_cond_signal(&cpool->ctx[i].cond);
            pthread_mutex_unlock(&cpool->ctx[i].mutex);
            pthread_join(cpool->threads[i], NULL);
            pthread_mutex_destroy(&cpool->ctx[i].mutex);
            pthread_cond_destroy(&cpool->ctx[i].cond);
            DBG("Thread %zu finalized", i);
        }

        pthread_mutex_destroy(&cpool->session_threads_mutex);

        // cleanup
        free(cpool->threads);
        free(cpool->thread_running);
    }
    free(cpool->ctx);

    free(cpool);
}

size_t cpool_least_populated_thread(SessionPool* cpool)
{
    size_t* count_per_thread = CALLOC(cpool->n_threads, sizeof(size_t));

    for (SessionThread* conn_th = cpool->session_thread_map; conn_th != NULL; conn_th = conn_th->hh.next)
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

void spool_add_session(SessionPool* cpool, Session* session)
{
    if (cpool->n_threads != SINGLE_THREADED) {
        // find least populated thread
        size_t thread_n = cpool_least_populated_thread(cpool);

        // add session to thread
        SessionThread* ct = MALLOC(sizeof *ct);
        ct->thread_n = thread_n;
        ct->session = session;
        ct->ready = false;
        pthread_mutex_lock(&cpool->session_threads_mutex);
        HASH_ADD_PTR(cpool->session_thread_map, session, ct);
        pthread_mutex_unlock(&cpool->session_threads_mutex);

        DBG("New session added to thread %zu", thread_n);
    }
}

void spool_remove_session(SessionPool* cpool, Session* session)
{
    if (cpool->n_threads != SINGLE_THREADED) {
        pthread_mutex_lock(&cpool->session_threads_mutex);
        SessionThread* st;
        HASH_FIND_PTR(cpool->session_thread_map, &session, st);
        if (st) {
            size_t thread_n = st->thread_n;
            pthread_mutex_lock(&cpool->ctx[thread_n].mutex);
            DBG("Session removed from thread %zu", st->thread_n);
            HASH_DEL(cpool->session_thread_map, st);
            free(st);
            pthread_mutex_unlock(&cpool->ctx[thread_n].mutex);
        }
        pthread_mutex_unlock(&cpool->session_threads_mutex);
    }
}

void spool_flush_session(SessionPool* cpool, Session* session)
{
    if (cpool->n_threads != SINGLE_THREADED) {
        SessionThread* ct;
        HASH_FIND_PTR(cpool->session_thread_map, &session, ct);
        if (ct) {
            pthread_mutex_lock(&cpool->session_threads_mutex);
            ct->ready = true;
            pthread_mutex_unlock(&cpool->session_threads_mutex);

            pthread_mutex_lock(&cpool->ctx[ct->thread_n].mutex);
            cpool->ctx[ct->thread_n].should_wake = true;
            pthread_cond_signal(&cpool->ctx[ct->thread_n].cond);
            pthread_mutex_unlock(&cpool->ctx[ct->thread_n].mutex);
        }
    } else {
        server_process_session(cpool->server, session);
    }
}