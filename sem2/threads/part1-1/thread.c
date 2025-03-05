#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#define N 5

int global = 10;

void *mythread(void *arg) {
	global++;
	int local = 1;
	static int local_static = 2;
	const int local_const = 3;
	local_static++;
	printf("\nmythread [%d %d %d]: Hello from mythread!\n<%d>\nlocal: %p -> %d\nlocal const: %p -> %d\nlocal static: %p -> %d\nglobal: %p -> %d\n",
		getpid(), getppid(), gettid(), pthread_equal(pthread_self(), *(pthread_t*)arg), &local,
		local, &local_const, local_const, &local_static, local_static, &global, global);
	//sleep(10000);
	return NULL;
}

int main() {
	pthread_t tid[N];
	int err;
	printf("         [PID  PPID  TID]\n");

	printf("main     [%d %d %d]: Hello from main!\n", getpid(), getppid(), gettid());

	for (int i = 0; i < N; ++i) {
		err = pthread_create(&tid[i], NULL, mythread, (void*)&tid[i]);
		if (err) {
	    	printf("main: pthread_create() failed: %s\n", strerror(err));
			return -1;
		}
	}

	for (int i = 0; i < N; ++i) {
		err = pthread_join(tid[i], NULL);
    		if (err) {
        		printf("main: pthread_join() failed: %s\n", strerror(err));
        		return -1;
    		}
    }

	return 0;
}