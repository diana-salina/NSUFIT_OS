#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>

void handle_sigint(int sig) {
    printf("Поток 2: Получен сигнал %d.\n", sig);
}

void* thread1_function(void* arg) {
    printf("Поток 3: Ожидание сигнала SIGQUIT...\n");
    sigset_t set;
    int sig;
    sigemptyset(&set);
    sigaddset(&set, SIGQUIT);
    sigwait(&set, &sig);
    printf("Поток 3: Получен сигнал SIGQUIT.\n");
    return NULL;
}

void* thread2_function(void* arg) {
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGQUIT);
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_sigint;
    sa.sa_mask = set;
    printf("Поток 2: Ожидание сигнала SIGINT (Ctrl+C)...\n");
    sigaction(SIGINT, &sa, 0);
    sigaction(SIGQUIT, &sa, 0);
    while(1) {
        sleep(1);
    }
    return NULL;
}


int main() {
    pthread_t thread1, thread2, thread3;

    printf("         [PID  PPID  TID]\n");
    printf("main     [%d %d %d]: Hello from main!\n", getpid(), getppid(), gettid());

    pthread_create(&thread1, NULL, thread1_function, NULL);
    pthread_create(&thread2, NULL, thread2_function, NULL);

    sigset_t set;
    sigfillset(&set);
    pthread_sigmask(SIG_BLOCK, &set, NULL);
    printf("Поток 1: Все сигналы заблокированы.\n");

    pthread_join(thread2, NULL);
    pthread_join(thread1, NULL);

    return 0;
}