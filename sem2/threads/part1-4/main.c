#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

void *mythread(void *arg) {
	printf("mythread [%d %d %d]\n", getpid(), getppid(), gettid());
	while(1) {
		printf("ha");
	}
	return NULL;
}

int main() {
	int err;
	printf("         [PID  PPID  TID]\n");
	printf("main     [%d %d %d]: Hello from main!\n", getpid(), getppid(), gettid());

	pthread_t tid;
	err = pthread_create(&tid, NULL, mythread, NULL);
	if (err) {
   		printf("main: pthread_create() failed: %s\n", strerror(err));
		return -1;
	}
	sleep(2);
	pthread_cancel(tid);
	err = pthread_join(tid, NULL);
    if (err) {
        printf("main: pthread_join() failed: %s\n", strerror(err));
        return -1;
    }

	return 0;
}