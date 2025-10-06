#include "pthread.h"

int pthread_create(pthread_t* thread, const void* attr, void* (*start_routine)(void*), void* arg)
{
	(void)attr; // Unused parameter
	HANDLE hThread = CreateThread(
		NULL,               // default security attributes
		0,                  // use default stack size
		(LPTHREAD_START_ROUTINE)start_routine, // thread function name
		arg,               // argument to thread function
		0,                  // use default creation flags
		NULL);              // returns the thread identifier
	if (hThread == NULL) {
		return GetLastError(); // Return error code on failure
	}
	*thread = hThread;
	return 0; // Success
}

int pthread_join(pthread_t thread, void** retval)
{
	(void)retval; // Unused parameter
	DWORD result = WaitForSingleObject(thread, INFINITE);
	if (result != WAIT_OBJECT_0) {
		return GetLastError(); // Return error code on failure
	}
	CloseHandle(thread);
	return 0; // Success
}

