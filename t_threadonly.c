#include "kernelthread.h"

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>

#include <sys/types.h>
#include <sys/syscall.h>

kernelthread_t* tid[2];
void** value_ptrs[2];

int doSomeThing(void *arg) {
	pid_t x = syscall(__NR_gettid);
	for(int i = 0; i < 20; i++) {
		printf("%d: %d\n", x, i);
	}

	kernelthread_exit(NULL);
	return 0;
}

int main(void) {
	int i = 0;
	int err;

	while(i < 2)
	{
		err = kernelthread_create(&(tid[i]), NULL, &doSomeThing, NULL);
		if (err != 0)
			printf("\ncan't create thread :[%s]", strerror(err));
		i++;
	}

	kernelthread_join(tid[0], value_ptrs[0]);
	kernelthread_join(tid[1], value_ptrs[1]);
	printf("child processes finished; exiting...\n");
	return 0;
}
