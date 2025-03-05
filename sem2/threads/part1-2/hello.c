#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

void *mythread(void *arg) {
	printf("mythread [%d %d %d]: Hello from mythread!\n", getpid(), getppid(), gettid());
	//sleep(10000);
	char** res = malloc(sizeof(char*) * 12);
	*res = "Hello world";
	return res;
}

int main() {
	pthread_t tid;
	int err;
	printf("         [PID  PPID  TID]\n");

	printf("main     [%d %d %d]: Hello from main!\n", getpid(), getppid(), gettid());

	err = pthread_create(&tid, NULL, mythread, NULL);
	if (err) {
    	printf("main: pthread_create() failed: %s\n", strerror(err));
		return -1;
	}

	void* retval;
	err = pthread_join(tid, &retval);
    if (err) {
        printf("main: pthread_join() failed: %s\n", strerror(err));
        return -1;
    }
    printf("Value returned: %s\n", *(char**)retval);
    free(retval);

	return 0;
}