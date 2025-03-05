#define _GNU_SOURCE
#include "mylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
void *my_thread_function(void *arg) {
    printf("mythread [%d %d %d]\n", getpid(), getppid(), gettid());
    printf("Поток запущен с аргументом: %s\n", (char *)arg);
    return NULL;
}

int main() {
    mythread_t thread;
    char *arg = "Hi!";
    printf("         [PID  PPID  TID]\n");

    printf("main     [%d %d %d]: Hello from main!\n", getpid(), getppid(), gettid());


    if (mythread_create(&thread, my_thread_function, arg) == -1) {
        printf("Cannot create thread");
        return 1;
    }
    mythread_join(&thread, NULL);
    printf("Thread finished\n");
    return 0;
}