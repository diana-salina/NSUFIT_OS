#define MYLIB

typedef void *(*start_routine_t)(void *);

typedef struct {
    int id;
    start_routine_t start_routine;
    void *arg;
    void* retval;
    void* stack;
    int is_canceled;
} thread_struct;

typedef thread_struct* mythread_t;
int mythread_create(mythread_t *thread, start_routine_t routine, void *arg);
void mythread_join(mythread_t *thread, void** retval);