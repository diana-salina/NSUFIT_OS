#include "mysync.h"
#include <stdio.h>
#include <pthread.h>


int shared_counter = 0;
mutex_t m;
spin_t s;

void *thread_func(void *arg) {
    for (int i = 0; i < 100000; i++) {
        mutex_lock(&m);
        //spin_lock(&s);
        shared_counter++;
        mutex_unlock(&m);
        //spin_unlock(&s);
    }
    return NULL;
}

int main() {
    pthread_t threads[10];

    mutex_init(&m);
    //spin_init(&s);

    for (int i = 0; i < 10; i++) {
        if (pthread_create(&threads[i], NULL, thread_func, NULL) != 0) {
            perror("pthread_create");
            return -1;
        }
    }
    //mutex_unlock(&m);

    for (int i = 0; i < 10; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("res: %d\n", shared_counter);


    return 0;
}