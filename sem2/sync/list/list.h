#ifndef ___LIST_H___
#define ___LIST_H___

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

#define MAX_LEN 100

typedef struct _Node {
	char value[MAX_LEN];
	struct _Node *next;
	pthread_mutex_t sync;
} node_t;

typedef struct _List {
	node_t *first;
	pthread_mutex_t sync;
} list_t;

list_t* list_init();
void list_destroy(list_t* list);
void list_insert(list_t* list, const char* str);
void list_swap(node_t* a, node_t* b);

#endif		// ___LIST_H___