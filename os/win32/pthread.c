#include "pthread.h"

#include <windows.h>
#include <stdlib.h>
#include <errno.h>

typedef HANDLE pthread_t;
typedef CRITICAL_SECTION pthread_mutex_t;
typedef CONDITION_VARIABLE pthread_cond_t;

/* --- Thread --- */

struct thread_start {
    void* (*start_routine)(void*);
    void* arg;
};

static DWORD WINAPI thread_trampoline(LPVOID param)
{
    struct thread_start* t = (struct thread_start*)param;
    void* ret = t->start_routine(t->arg);
    free(t);
    return (DWORD)(uintptr_t)ret;
}

int pthread_create(pthread_t* thread, const void* attr,
    void* (*start_routine)(void*), void* arg)
{
    (void)attr;

    struct thread_start* t = malloc(sizeof(*t));
    if (!t)
        return ENOMEM;

    t->start_routine = start_routine;
    t->arg = arg;

    HANDLE h = CreateThread(NULL, 0, thread_trampoline, t, 0, NULL);
    if (!h) {
        free(t);
        return (int)GetLastError();
    }

    *thread = h;
    return 0;
}

int pthread_join(pthread_t thread, void** retval)
{
    DWORD rc = WaitForSingleObject(thread, INFINITE);
    if (rc != WAIT_OBJECT_0)
        return (int)GetLastError();

    if (retval) {
        DWORD code = 0;
        GetExitCodeThread(thread, &code);
        *retval = (void*)(uintptr_t)code;
    }

    CloseHandle(thread);
    return 0;
}

/* --- Mutex --- */

int pthread_mutex_init(pthread_mutex_t* mutex, const void* attr)
{
    (void)attr;
    InitializeCriticalSection(mutex);
    return 0;
}

int pthread_mutex_lock(pthread_mutex_t* mutex)
{
    EnterCriticalSection(mutex);
    return 0;
}

int pthread_mutex_unlock(pthread_mutex_t* mutex)
{
    LeaveCriticalSection(mutex);
    return 0;
}

int pthread_mutex_destroy(pthread_mutex_t* mutex)
{
    DeleteCriticalSection(mutex);
    return 0;
}

/* --- Condition variable --- */

int pthread_cond_init(pthread_cond_t* cond, const void* attr)
{
    (void)attr;
    InitializeConditionVariable(cond);
    return 0;
}

int pthread_cond_signal(pthread_cond_t* cond)
{
    WakeConditionVariable(cond);
    return 0;
}

int pthread_cond_wait(pthread_cond_t* cond, pthread_mutex_t* mutex)
{
    BOOL ok = SleepConditionVariableCS(cond, mutex, INFINITE);
    if (!ok)
        return (int)GetLastError();
    return 0;
}

int pthread_cond_destroy(pthread_cond_t* cond)
{
    (void)cond; /* CONDITION_VARIABLE doesn't need cleanup */
    return 0;
}
