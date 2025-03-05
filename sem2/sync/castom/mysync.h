#ifndef __MYSYNC__
#define __MYSYNC__

#include <stdatomic.h>
#include <stdlib.h>

typedef struct Spinlock {
	atomic_flag lock;
} spin_t;

void spin_init(spin_t* m);
void spin_lock(spin_t* m);
void spin_unlock(spin_t* m);

typedef struct Mutex{
	atomic_flag lock;
	pid_t tid;
} mutex_t;

void mutex_init(mutex_t* m);
void mutex_lock(mutex_t* m);
void mutex_unlock(mutex_t* m);



#endif //__MYSYNC__