#include "kernelthread.h"

#include<stdio.h>
#include<string.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>

#include <sys/types.h>
#include <sys/syscall.h>

kernelthread_t* tid[4];
int counter;
kernelthread_mutex_t* lock;

int doSomeThing(void *arg) {
	pid_t x = syscall(__NR_gettid);
	for(int i = 0; i < 10; i++) {
		printf("%d: %d\n", x, i);
	}

	kernelthread_mutex_lock(lock);

	int i = 0;
	counter += 1;
	printf("Job %d started\n", counter);

	for(i=0; i<20;i++){
		printf("%d working: %d\n", x, i);
	}

	printf("Job %d finished\n", counter);

	kernelthread_mutex_unlock(lock);

	for(int i = 25; i < 30; i++) {
		printf("%d: %d\n", x, i);
	}

	return 0;
}

int main(void) {
	int i = 0;
	int err;

	if (kernelthread_mutex_init(&lock) != 0) {
		printf("\n mutex init failed\n");
		return 1;
	}

	while(i < 4) {
		err = kernelthread_create(&(tid[i]), NULL, &doSomeThing, NULL);
		if (err != 0)
			printf("\ncan't create thread :[%s]", strerror(err));
		i++;
	}

	kernelthread_join(tid[0], NULL);
	kernelthread_join(tid[1], NULL);
	kernelthread_join(tid[2], NULL);
	kernelthread_join(tid[3], NULL);

	kernelthread_mutex_destroy(lock);

	return 0;
}
