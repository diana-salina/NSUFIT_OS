#define _GNU_SOURCE
#include "mysync.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/futex.h>
#include <syscall.h>
#include <errno.h>
#include <sched.h>

void spin_init(spin_t* m) {
	atomic_flag_clear(&m->lock);
}

void spin_lock(spin_t* m) {
	while(atomic_flag_test_and_set(&m->lock)) {
		sched_yield();
	}
}

void spin_unlock(spin_t* m) {
	atomic_flag_clear(&m->lock);

}

void mutex_init(mutex_t* m) {
	atomic_flag_clear(&m->lock);
	m->tid = 0;
}

void mutex_lock(mutex_t* m) {
	pid_t current_tid = syscall(SYS_gettid);
	while (atomic_flag_test_and_set(&m->lock)) {
		if (syscall(SYS_futex, &m->lock, FUTEX_WAIT, 0, NULL, NULL, 0) == -1 && errno != EAGAIN) {
			printf("futex_wait failed\n");
			abort();
		}
	}

	m->tid = current_tid;
}

void mutex_unlock(mutex_t* m) {
	pid_t current_tid = syscall(SYS_gettid);
	if (m->tid != current_tid) {
        printf("INVALID MUTEX UNLOCK: Current thread does not own the mutex\n");
        abort();
    }
    m->tid = 0;

	atomic_flag_clear(&m->lock);
	if (syscall(SYS_futex, &m->lock, FUTEX_WAKE, 1, NULL, NULL, 0) == -1) {
		printf("futex-wake failed\n");
		abort();
	}
}

