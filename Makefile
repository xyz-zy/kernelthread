# compiler to use
CC = gcc-4.9

# flags to pass compiler
CFLAGS = -ggdb3 -O0 -std=gnu11 -Wall -Werror

# name for executable
EXE = t_threadmutex t_threadonly t_mutexonly

# default target

all: t_mutexonly t_threadonly t_threadmutex

# housekeeping
clean:
	rm -f core $(EXE) *.o *.d *.out


t_mutexonly:
	$(CC) $(CFLAGS) -o t_mutexonly t_mutexonly.c kernelthread.c -lpthread

t_threadonly:
	$(CC) $(CFLAGS) -o t_threadonly t_threadonly.c kernelthread.c

t_threadmutex:
	$(CC) $(CFLAGS) -o t_threadmutex t_threadmutex.c kernelthread.c
