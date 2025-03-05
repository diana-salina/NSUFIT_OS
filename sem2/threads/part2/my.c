#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sched.h>
#include <string.h>
#include <pthread.h>
#include <linux/futex.h>
#include <syscall.h>


typedef void *(*start_routine_t)(void *);

typedef struct {
    int id;
    start_routine_t start_routine;
    void *arg;
    void* retval;
    void* stack;
    int is_finished;
    int is_joined;
    int futex;
} thread_struct;

typedef thread_struct* mythread_t;
static int ids = 0;

int thread_function(void *arg) {
    mythread_t mythread = (mythread_t)arg;
    mythread->retval = mythread->start_routine(mythread->arg);
    mythread->is_finished = 1;
    syscall(SYS_futex, &mythread->futex, FUTEX_WAKE, 1, NULL, NULL, 0);

    while (!mythread->is_joined) {
        syscall(SYS_futex, &mythread->futex, FUTEX_WAIT, 0, NULL, NULL, 0);
    }
    return 0;
}

int mythread_create(mythread_t *thread, start_routine_t routine, void *arg) {
    int STACK_SIZE = getpagesize() * 4;
    ids++;
    void* region = mmap(NULL, STACK_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (region == MAP_FAILED) {
        printf("mmap failed\n");
        ids--;
        return -1;
    }

    mythread_t t = (mythread_t)((char*)region + STACK_SIZE - sizeof(thread_struct));
    t->id = ids;
    t->start_routine = routine;
    t->arg = arg;
    t->retval = NULL;
    t->stack = region;
    t->is_finished = 0;
    t->is_joined = 0;
    t->futex = 0;

    int pid = clone(thread_function, (void*)t,CLONE_VM | CLONE_SIGHAND | CLONE_FILES | CLONE_FS | CLONE_THREAD | CLONE_PARENT_SETTID, t);
    if (pid == -1) {
        printf("clone failed\n");
        munmap(region, STACK_SIZE);
        return -1;
    }
    *thread = t;
    return 0;
}

void mythread_join(mythread_t *thread, void** retval) {
    while (!(*thread)->is_finished) {
        syscall(SYS_futex, &(*thread)->futex, FUTEX_WAIT, 1, NULL, NULL, 0);
    }
    (*thread)->is_joined = 1;
    if (retval != NULL) {
        *retval = (*thread)->retval;
    }

    (*thread)->futex = 0;
    syscall(SYS_futex, &(*thread)->futex, FUTEX_WAKE, 0, NULL, NULL, 0);
    munmap((*thread)->stack, getpagesize()*4);
    printf("thread joined\n");
}