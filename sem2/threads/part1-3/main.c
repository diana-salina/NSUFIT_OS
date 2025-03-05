#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

struct my {
	int a;
	char* b;
} typedef my;

void *mythread(void *arg) {
	my val = *(my*)arg;
	printf("mythread [%d %d %d]: {%d, %s} \n", getpid(), getppid(), gettid(), val.a, val.b);
	//sleep(10000);
	return NULL;
}

int main() {
	pthread_t tid;
	int err;
	printf("         [PID  PPID  TID]\n");
	printf("main     [%d %d %d]: Hello from main!\n", getpid(), getppid(), gettid());

	my* val = malloc(sizeof(my));
	val->a = 5;
	val->b = "hello";

	err = pthread_create(&tid, NULL, mythread, (void*)val);
	if (err) {
		free(val);
    	printf("main: pthread_create() failed: %s\n", strerror(err));
		return -1;
	}

	err = pthread_join(tid, NULL);
   	if (err) {
   		free(val);
        printf("main: pthread_join() failed: %s\n", strerror(err));
      	return -1;
   	}
   	free(val);

	return 0;
}