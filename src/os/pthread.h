#ifndef PTHREAD_H_
#define PTHREAD_H_

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

typedef HANDLE pthread_t;

int pthread_create(pthread_t* thread, const void* attr, void* (*start_routine)(void*), void* arg);
int pthread_join(pthread_t thread, void** retval);

#endif
