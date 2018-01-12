#include "kernelthread.h"

/* comments from pthread documentation at http://www.cse.iitd.ernet.in/~dheerajb/Pthreads/Document/Pthreads_Information.html */

//////////////// KERNELTHREAD ARRAY ////////////////

kernelthread_t* thread_array[MAX_THREADS];
kernelthread_mutex_t* thread_mutex;

int find_avail_thread() {
	for(int i = 0; i < MAX_THREADS; i++) {
		if(thread_array[i] == 0 || thread_array[i]->status == 0 || thread_array[i]->status == -1) {
			return i;
		}
	}
	return -1;
} 

//constructor called before main() begins -- initialize mutex for use in kernelthread functions
__attribute__((constructor)) void kernelthread_init() {
	kernelthread_mutex_init(&thread_mutex);
}

//destructor called once main() exits -- free kernelthread resources to prevent memory leaks
__attribute__((destructor)) void wait_and_free_all() {
//	printf("destructor!\n");
	for(int i = 0; i < MAX_THREADS; i++) {
		if(thread_array[i] != 0) {
			if(thread_array[i]->status == -1) {
				free(thread_array[i]->stack - STACK_SIZE);
			} else if(thread_array[i]->status == 1) {
//				printf("destructor wait!\n");
				waitpid(thread_array[i]->pid, NULL, 0);
				free(thread_array[i]->stack - STACK_SIZE);
			}
			free(thread_array[i]);
			thread_array[i] = 0;
		}
	}
	kernelthread_mutex_destroy(thread_mutex);
//	printf("destructor end!\n");
}

//////////////// THREAD FUNCTIONS /////////////////

//Creates a new thread, initializes its attributes, and makes it runnable
int kernelthread_create(kernelthread_t** thread, const kernelthread_attr_t *attr, int (*start_routine) (void *), void *arg){

	kernelthread_mutex_lock(thread_mutex);

	kernelthread_t* new_thread;
	int thread_num = find_avail_thread();

	if(thread_num == -1) {
		fprintf(stderr, "error: maximum number of threads reached\n");
		return 1;
	}
	
	if(thread_array[thread_num] == 0) {
		new_thread = malloc(sizeof(kernelthread_t));
		new_thread->stack = (void*) malloc(STACK_SIZE) + STACK_SIZE;
	} else {
		new_thread = thread_array[thread_num];
		if(new_thread->stack == 0) {	
			new_thread->stack = (void*) malloc(STACK_SIZE) + STACK_SIZE;
		}
	}

	if(new_thread->stack == 0) {
		fprintf(stderr, "error: could not allocate stack for new thread\n");
		return 1;
	}

	new_thread->parent_pid = getpid();
	new_thread->status = 1;

	thread_array[thread_num] = new_thread;

	thread_array[thread_num]->pid = clone(start_routine, (char*) thread_array[thread_num]->stack, SIGCHLD | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_VM, arg);

	if(thread_array[thread_num]->pid == -1) {
		free(thread_array[thread_num]->stack);
		thread_array[thread_num]->status = 0;
		fprintf(stderr, "error: clone() system call failed\n");
		return 1;
	}
//	printf("sp: %p\n", thread_array[thread_num]->stack);
	*thread = thread_array[thread_num];
	
//	printf("sp: %p\n", thread->stack);
	
	kernelthread_mutex_unlock(thread_mutex);
	return 0;

}

//Terminates the calling thread
void kernelthread_exit(void *value_ptr) {
	kernelthread_t this = kernelthread_self();
	atomic_exchange(&((&this)->status), -1);
	syscall(SYS_exit);
}

//Returns the thread's identifier
kernelthread_t kernelthread_self() {
	kernelthread_mutex_lock(thread_mutex);
	kernelthread_t* found = NULL;
	pid_t pid = getpid();
	for(int i = 0; i < MAX_THREADS; i++) {
		if(thread_array[i] != 0) {
			if(thread_array[i]->pid == pid) {
				found = thread_array[i];
				break;
			}
		}
	}
	kernelthread_mutex_unlock(thread_mutex);
	return *found;
}

//Blocks the calling thread until the thread specified in the call terminates. The target thread's termination status is returned in the status parameter.
//acts like pthread_cond_wait soubroutine to wait for a special condition?!?
int kernelthread_join(kernelthread_t* thread, void **value_ptr) {
	
//	printf("join sp:%p\n", (&thread)->stack);
//	printf("trying to find segfault0\n");
	waitpid(thread->pid, (int*) value_ptr, 0);
//	printf("trying to find segfault1\n");
	free(thread->stack - STACK_SIZE);
//	printf("trying to find segfault2\n");
	thread->stack = 0;
	thread->status = 0;
//	printf("trying to find segfault3\n");
	return 0;
}

/* not sure how to make OS cleanup after detached thread
//Detaches the specified thread from the calling thread
int kernelthread_detach(kernelthread_t thread, void **value_ptr);
*/

/////// LINKED LIST OF INITIALIZED MUTEXES ////////

typedef struct mutex_node {
	kernelthread_mutex_t* mutex;
	struct mutex_node* prev;
	struct mutex_node* next;
} mutex_node;

mutex_node* head;
mutex_node* tail;

mutex_node* find_mutex_node(kernelthread_mutex_t* mutex) {
	mutex_node* cur_node = head;
	while(cur_node != 0) {
		if(cur_node->mutex == mutex) {
			return cur_node;
		} else {
			cur_node = cur_node->next;
		}
	}
	
	return 0;
}

///////////////// MUTEX FUNCTIONS /////////////////

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
int kernelthread_mutex_init(kernelthread_mutex_t** mutex) {
	*mutex = (kernelthread_mutex_t*) calloc(1, sizeof(kernelthread_mutex_t));
	mutex_node* new_node = calloc(1, sizeof(mutex_node));
	new_node->mutex = *mutex;
	if(head == 0) {
		head = tail = new_node;
	} else {
		tail->next = new_node;
		new_node->prev = tail;
		tail = new_node;
	}
	return 0;
}

//Locks a mutex
int kernelthread_mutex_lock(kernelthread_mutex_t* mutex) {
	
	mutex_node* node = find_mutex_node(mutex);
	if(node == 0) {			//error: mutex not initialized
		return EINVAL;
	}	

	pid_t x = syscall(__NR_gettid);
	if(mutex->pid == x) {	//error: current thread already owns mutex
		return EDEADLK;
	}

	/*
		lock can have 3 states:
			0 - lock is free, can be acquired
			1 - lock is being held, cannot be acquired
			2 - lock is being held, and another thread is waiting to acquire
	*/

	int c = 0;
	//try to grab lock
	atomic_compare_exchange_weak(&(mutex->turn), &c, 1);
	//if failed (lock was not free, so c != 0), then wait
	if(c != 0) {
		//if c == 1, change to 2 because we have a thread waiting to acquire
		if(c != 2) {
			c = atomic_exchange(&(mutex->turn), 2);
		} 
		//wait to acquire while checking lock
		while (c != 0) {
			syscall(SYS_futex, &(mutex->turn), FUTEX_WAIT, 2, NULL, NULL, 0);
			c = atomic_exchange(&(mutex->turn), 2);
		}
	}

	mutex->pid = x;

	return 0;
}

//Tries to lock a mutex. If the mutex is currently locked (by any thread, including calling thread), then returns immediately.
int kernelthread_mutex_trylock(kernelthread_mutex_t* mutex) {

	mutex_node* node = find_mutex_node(mutex);
	if(node == 0) {			//error: mutex not initialized
		return EINVAL;
	}

	pid_t x = syscall(__NR_gettid);
	int c = 0;
	atomic_compare_exchange_weak(&(mutex->turn), &c, 1);
	if(c == 0) {
		mutex->pid = x;
		return 0;
	} else {
		return EBUSY;
	}
}

//Unlocks a mutex
int kernelthread_mutex_unlock(kernelthread_mutex_t* mutex) {

	mutex_node* node = find_mutex_node(mutex);
	if(node == 0) {			//error: mutex not initialized
		return EINVAL;
	}

	pid_t x = syscall(__NR_gettid);
	if(mutex->pid != x) {	//error: current thread does not own mutex
		return EPERM;
	}

	mutex->pid = 0;

	if(atomic_fetch_sub(&(mutex->turn), 1) != 1) {
		atomic_exchange(&(mutex->turn), 0);	
		//wake up waiting thread	
		syscall(SYS_futex, &(mutex->turn), FUTEX_WAKE, 1, NULL, NULL, 0);
	}

	return 0;
}

//Deletes a mutex
void kernelthread_mutex_destroy(kernelthread_mutex_t* mutex) {

	mutex_node* node = find_mutex_node(mutex);
	if(node == 0) {		//error: mutex not initialzed
		return;
	}
	
	if(node->prev == 0) {
		head = node->next;
	} else {
		node->prev->next = node->next;
	}

	free(mutex);
	free(node);
}
