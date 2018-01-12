# kernelthread
A basic C library implementation of threads and mutexes. For learning purposes.

The following functions are implemented(see kernelthread.h for a description of each): 

	int kernelthread_create()
	void kernelthread_exit()
	kenelthread_t kernelthread_self()
	int kernelthread_join()

	kernelthread_mutex_init()
	kernelthread_mutex_lock()
	kernelthread_mutex_trylock()
	kernelthread_mutex_unlock()
	kernelthread_mutex_destroy()

Quick overview:

  1) The maximum number of kernelthreads that can be spawned is defined by MAX_THREADS in kernel.h (currently 10). This a very conservative upper bound; in reality this is limited by the heap size, which determines the number of kernelthread stacks that can be allocated.
  
  2) A stack of size STACK_SIZE (bytes) is allocated for each new thread. This is currently 1MB.
  
  3) The fini/destructor function wait_and_free_all() ensures that the main thread will wait for all unjoined kernelthreads to terminate before exiting (even after main() returns).
  
  4) For some reason, there is a bug where kernelthreads will randomly segfault during the tests (about 1/7th to 1/10th chance). I spent hours trying to diagnose this, but I couldn't get gdb to examine the execution of different threads, and valgrind slows down program execution too much to reproduce the bug--the bug disappears when running the tests in valgrind.

  5) The logic for the mutex implementation was found in a paper titled "Futexes are Tricky" (Citation VII). I found this to be very helpful in understanding the problems and race conditions associated with mutex design. Understanding the paper, figuring out system calls and atomic instructions, and actually implementing and debugging the mutex took 9-11 hours.

  6) Basic error checking is implemented for mutexes: EBUSY, EINVAL, EAGAIN, EDEADLCK, EPERM. Details about when these errors occur can be found in kernelthread.h
  
References:

I)	pthread & pthread_mutex example:
	http://www.thegeekstuff.com/2012/05/c-mutex-examples/?refcom

II)	ways to implement basic thread libraries:
	http://www.evanjones.ca/software/threading.html

III)Peterson's Algorithm:
	https://en.wikipedia.org/wiki/Peterson%27s_algorithm

IV)	pthread_mutex documentation:
	http://pubs.opengroup.org/onlinepubs/7908799/xsh/pthread_mutex_init.html

V)	Writing a simple lock (mutex) in C/Assembly:
	http://forum.codecall.net/topic/64301-writing-a-simple-lock-mutex-in-cassembly/

VI)	futex:
	https://linux.die.net/man/2/futex

VII)futexes are tricky:
	https://www.akkadia.org/drepper/futex.pdf

VIII)install gcc-4.9:

https://askubuntu.com/questions/466651/how-do-i-use-the-latest-gcc-on-ubuntu?answertab=active#tab-top

	sudo add-apt-repository ppa:ubuntu-toolchain-r/test
	sudo apt-get update
	sudo apt-get install gcc-4.9 g++-4.9

IX) explanation of basic pthread functions:
	http://www.cse.iitd.ernet.in/~dheerajb/Pthreads/Document/Pthreads_Information.html

X) constructors/destructors:
	https://phoxis.org/2011/04/27/c-language-constructors-and-destructors-with-gcc/

XI) Linux man pages

XII) pthread source code
	http://cvsweb.netbsd.org/bsdweb.cgi/src/lib/libpthread/?only_with_tag=MAIN#dirlist

XIII) user space threads
	http://web.cs.ucla.edu/classes/winter17/cs111/readings/user_threads.html
	https://www.cs.utexas.edu/users/dahlin/Classes/UGOS/labs/labULT/proj-ULT.html
	https://people.cs.clemson.edu/~jsorber/courses/cpsc3220-S17/projects/project_2.pdf
