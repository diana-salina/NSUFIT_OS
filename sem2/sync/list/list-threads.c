#include "list.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define LIST_SIZE 1000

typedef struct _Stat {
	int inc_iters;
	int dec_iters;
	int eq_iters;
	int swap_attempts;

	pthread_spinlock_t inc_sync;
	pthread_spinlock_t dec_sync;
	pthread_spinlock_t eq_sync;
	pthread_spinlock_t swap_sync;
} stat_t;

stat_t stat;

void stat_init(stat_t* stat) {
	stat->inc_iters = 0;
	stat->dec_iters = 0;
	stat->eq_iters = 0;
	stat->swap_attempts = 0;

	pthread_spin_init(&stat->inc_sync, PTHREAD_PROCESS_PRIVATE);
	pthread_spin_init(&stat->dec_sync, PTHREAD_PROCESS_PRIVATE);
	pthread_spin_init(&stat->eq_sync, PTHREAD_PROCESS_PRIVATE);
	pthread_spin_init(&stat->swap_sync, PTHREAD_PROCESS_PRIVATE);
}

void stat_close(stat_t* stat) {
	pthread_spin_destroy(&stat->inc_sync);
	pthread_spin_destroy(&stat->dec_sync);
	pthread_spin_destroy(&stat->eq_sync);
	pthread_spin_destroy(&stat->swap_sync);
}

void* inc_thread(void* arg) {
	list_t* list = (list_t*)arg;
	while(1) {
		pthread_mutex_lock(&list->first->sync);
        node_t* current = list->first;
        pthread_mutex_unlock(&list->first->sync);

		int count = 0;
		//printf("cur: %s\n", current->value);
		while(current && current->next) {
			pthread_testcancel();
			pthread_mutex_lock(&current->sync);
			pthread_mutex_lock(&current->next->sync);

			if (strlen(current->value) < strlen(current->next->value)) {
				++count;
				//printf("inc: %d\n", count);
			}

			pthread_mutex_unlock(&current->sync);
			pthread_mutex_unlock(&current->next->sync);
			/*
			if (pthread_mutex_trylock(&current->sync) == 0) {
				if (pthread_mutex_trylock(&current->next->sync) == 0) {
					//printf("%s %s\n", current->value, current->next->value);

					if (strlen(current->value) < strlen(current->next->value)) {
						++count;
						//printf("inc: %d\n", count);
					}

					pthread_mutex_unlock(&current->next->sync);
				}
				pthread_mutex_unlock(&current->sync);
			}
			*/
			current = current->next;
		}
		//printf("aaa\n");
		pthread_spin_lock(&stat.inc_sync);
		stat.inc_iters++;
		pthread_spin_unlock(&stat.inc_sync);
	}
	return NULL;
}

void* dec_thread(void* arg) {
	list_t* list = (list_t*)arg;
	while(1) {
		pthread_mutex_lock(&list->first->sync);
        node_t* current = list->first;
        pthread_mutex_unlock(&list->first->sync);
		int count = 0;
		while(current && current->next) {
			pthread_testcancel();
			pthread_mutex_lock(&current->sync);
			pthread_mutex_lock(&current->next->sync);

			if (strlen(current->value) > strlen(current->next->value)) {
				++count;
				//printf("inc: %d\n", count);
			}

			pthread_mutex_unlock(&current->sync);
			pthread_mutex_unlock(&current->next->sync);
			/*
			if (pthread_mutex_trylock(&current->sync) == 0) {
				if (pthread_mutex_trylock(&current->next->sync) == 0) {
					//printf("%s %s\n", current->value, current->next->value);

					if (strlen(current->value) > strlen(current->next->value)) {
						++count;
						//printf("inc: %d\n", count);
					}

					pthread_mutex_unlock(&current->next->sync);
				}
				pthread_mutex_unlock(&current->sync);
			}
			*/
			current=current->next;
		}
		pthread_spin_lock(&stat.dec_sync);
		stat.dec_iters++;
		pthread_spin_unlock(&stat.dec_sync);
	}
	return NULL;

}

void* eq_thread(void* arg) {
	list_t* list = (list_t*)arg;
	while(1) {
		pthread_mutex_lock(&list->first->sync);
        node_t* current = list->first;
        pthread_mutex_unlock(&list->first->sync);
		int count = 0;
		while(current && current->next) {
			pthread_testcancel();
			pthread_mutex_lock(&current->sync);
			pthread_mutex_lock(&current->next->sync);

			if (strlen(current->value) == strlen(current->next->value)) {
				++count;
				//printf("inc: %d\n", count);
			}

			pthread_mutex_unlock(&current->sync);
			pthread_mutex_unlock(&current->next->sync);
			/*
			if (pthread_mutex_trylock(&current->sync) == 0) {
				if (pthread_mutex_trylock(&current->next->sync) == 0) {
					//printf("%s %s\n", current->value, current->next->value);

					if (strlen(current->value) == strlen(current->next->value)) {
						++count;
						//printf("inc: %d\n", count);
					}

					pthread_mutex_unlock(&current->next->sync);
				}
				pthread_mutex_unlock(&current->sync);
			}
			*/
			current=current->next;
		}
		pthread_spin_lock(&stat.eq_sync);
		stat.eq_iters++;
		pthread_spin_unlock(&stat.eq_sync);
	}
	return NULL;

}

void* swap_thread(void* arg) {
	list_t* list = (list_t*)arg;
	srand(time(NULL));
	while(1) {
		pthread_mutex_lock(&list->first->sync);
        node_t* current = list->first;
        pthread_mutex_unlock(&list->first->sync);
		while (current && current->next) {
			pthread_testcancel();
			pthread_mutex_lock(&current->sync);
			pthread_mutex_lock(&current->next->sync);

			if (rand() % 2) {
				list_swap(current, current->next);
				pthread_spin_lock(&stat.swap_sync);
				stat.swap_attempts++;
				pthread_spin_unlock(&stat.swap_sync);
			}

			pthread_mutex_unlock(&current->sync);
			pthread_mutex_unlock(&current->next->sync);
			/*
			if (pthread_mutex_trylock(&current->sync) == 0) {
				if (pthread_mutex_trylock(&current->next->sync) == 0) {
					if (rand() % 2) {
						list_swap(current, current->next);
						pthread_spin_lock(&stat.swap_sync);
						stat.swap_attempts++;
						pthread_spin_unlock(&stat.swap_sync);
					}
					pthread_mutex_unlock(&current->next->sync);
				}
				pthread_mutex_unlock(&current->sync);
			}
			*/
			current = current->next;
		}
	}
	return NULL;
}

void* print_stat(void* arg) {
	while(1) {
		sleep(1);
		pthread_spin_lock(&stat.inc_sync);
		pthread_spin_lock(&stat.dec_sync);
		pthread_spin_lock(&stat.eq_sync);
		pthread_spin_lock(&stat.swap_sync);

		printf("inc_iters: %d; dec_iters: %d; eq_iters: %d; swap_attempts: %d\n",
			stat.inc_iters, stat.dec_iters, stat.eq_iters, stat.swap_attempts);

		pthread_spin_unlock(&stat.inc_sync);
		pthread_spin_unlock(&stat.dec_sync);
		pthread_spin_unlock(&stat.eq_sync);
		pthread_spin_unlock(&stat.swap_sync);
	}
	return NULL;
}

int main(void) {
	list_t* list = list_init();
	char* str = (char*)malloc(sizeof(char) * MAX_LEN);
	for(int i = 0; i < LIST_SIZE; ++i) {
		snprintf(str, MAX_LEN, "%d", i);
		list_insert(list, str);
	}
	free(str);

	//printf("*%s %s\n", list->first->value, list->first->next->value);

	stat_init(&stat);
	int err;
	pthread_t tid_inc, tid_dec, tid_eq;
	pthread_t tid_swap1, tid_swap2, tid_swap3, tid_print;

	err = pthread_create(&tid_inc, NULL, inc_thread, list);
	if (err) {
		printf("main: pthread_create() failed: %s\n", strerror(err));
	}

	err = pthread_create(&tid_dec, NULL, dec_thread, list);
	if (err) {
		printf("main: pthread_create() failed: %s\n", strerror(err));
	}

	err = pthread_create(&tid_eq, NULL, eq_thread, list);
	if (err) {
		printf("main: pthread_create() failed: %s\n", strerror(err));
	}

	err = pthread_create(&tid_swap1, NULL, swap_thread, list);
	if (err) {
		printf("main: pthread_create() failed: %s\n", strerror(err));
	}

	err = pthread_create(&tid_swap2, NULL, swap_thread, list);
	if (err) {
		printf("main: pthread_create() failed: %s\n", strerror(err));
	}

	err = pthread_create(&tid_swap3, NULL, swap_thread, list);
	if (err) {
		printf("main: pthread_create() failed: %s\n", strerror(err));
	}

	err = pthread_create(&tid_print, NULL, print_stat, NULL);
	if (err) {
		printf("main: pthread_create() failed: %s\n", strerror(err));
	}

	sleep(10);
	pthread_cancel(tid_inc);
    pthread_cancel(tid_dec);
    pthread_cancel(tid_eq);
    pthread_cancel(tid_swap1);
    pthread_cancel(tid_swap2);
    pthread_cancel(tid_swap3);
    pthread_cancel(tid_print);

	err = pthread_join(tid_inc, NULL);
	if (err) {
		printf("main: pthread_join() failed: %s\n", strerror(err));
	}

	err = pthread_join(tid_dec, NULL);
	if (err) {
		printf("main: pthread_join() failed: %s\n", strerror(err));
	}

	err = pthread_join(tid_eq, NULL);
	if (err) {
		printf("main: pthread_join() failed: %s\n", strerror(err));
	}

	err = pthread_join(tid_swap1, NULL);
	if (err) {
		printf("main: pthread_join() failed: %s\n", strerror(err));
	}

	err = pthread_join(tid_swap2, NULL);
	if (err) {
		printf("main: pthread_join() failed: %s\n", strerror(err));
	}

	err = pthread_join(tid_swap3, NULL);
	if (err) {
		printf("main: pthread_join() failed: %s\n", strerror(err));
	}

	err = pthread_join(tid_print, NULL);
	if (err) {
		printf("main: pthread_join() failed: %s\n", strerror(err));
	}


	list_destroy(list);
	stat_close(&stat);
	return 0;
}
