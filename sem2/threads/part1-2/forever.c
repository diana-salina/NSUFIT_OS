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
	return NULL;
}

int main() {
	int err;
	printf("         [PID  PPID  TID]\n");
	printf("main     [%d %d %d]: Hello from main!\n", getpid(), getppid(), gettid());

	while(1) {
		pthread_t tid;
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		err = pthread_create(&tid, NULL, mythread, NULL);
		//pthread_detach(tid);
		usleep(50000);
		if (err) {
   			printf("main: pthread_create() failed: %s\n", strerror(err));
			return -1;
		}
		pthread_attr_destroy(&attr);
	}

	return 0;
}