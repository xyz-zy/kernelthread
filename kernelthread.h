#define _GNU_SOURCE

#include <sched.h>
#include <stdlib.h>
#include <linux/futex.h>
#include <signal.h>
#include <sys/time.h>
#include <stdatomic.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdio.h>

/* comments from pthread documentation at http://www.cse.iitd.ernet.in/~dheerajb/Pthreads/Document/Pthreads_Information.html */

////////// THREAD //////////
#define STACK_SIZE (1024 * 1024)	//1MB
#define MAX_THREADS	10

#ifndef kernelthread_structs
#define kernelthread_structs
typedef struct kernelthread_t {
	int status;
	void* stack;
	pid_t parent_pid;
	pid_t pid;
} kernelthread_t;

typedef struct kernelthread_attr_t {

} kernelthread_attr_t;
#endif

//Creates a new thread, initializes its attributes, and makes it runnable
extern int kernelthread_create(kernelthread_t** thread, const kernelthread_attr_t *attr, int (*start_routine)(void*), void *arg);

//Terminates the calling thread
extern void kernelthread_exit(void *value_ptr);

//Returns the thread's identifier
extern kernelthread_t kernelthread_self();

//Blocks the calling thread until the thread specified in the call terminates. The target thread's termination status is returned in the status parameter.
//acts like pthread_cond_wait soubroutine to wait for a special condition?!?
extern int kernelthread_join(kernelthread_t* thread, void **value_ptr);
/*
//Detaches the specified thread from the calling thread
extern int kernelthread_detach(kernelthread_t thread, void **value_ptr);
*/
////////// MUTEX //////////

#ifndef kernelthread_mutex_structs
#define kenelthread_mutex_structs
typedef struct kernelthread_mutex_t{
	pid_t pid;
	int recursive_calls;
	atomic_int turn;
} kernelthread_mutex_t;
#endif

/*some of the pthread_mutex errors are checked for:
	trylock() will fail if: 
		EBUSY - mutex could not be acquired b/c already locked
	lock(), trylock(), and unlock() may fail if: 
		EINVAL - ptr does not refer to an initialized mutex obj
		EAGAIN - mutex could not be acquired b/c max recursive locks has been exceeded
	lock() may fail if:
		EDEADLCK - the current thread already owns the mutex
	unlock() may fail if:
		EPERM - current thread does not own the mutex
*/

//Initializes a mutex
extern int kernelthread_mutex_init(struct kernelthread_mutex_t**);

//Locks a mutex
extern int kernelthread_mutex_lock(struct kernelthread_mutex_t*);

//Tries to lock a mutex. If the mutex is currently locked (by any thread, including calling thread), then returns immediately.
extern int kernelthread_mutex_trylock(struct kernelthread_mutex_t*);

//Unlocks a mutex
extern int kernelthread_mutex_unlock(struct kernelthread_mutex_t*);

//Deletes a mutex
extern void kernelthread_mutex_destroy(struct kernelthread_mutex_t*);
