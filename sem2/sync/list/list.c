#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


list_t* list_init() {
	list_t* list = (list_t*)malloc(sizeof(list_t));
	list->first = NULL;
	pthread_mutex_init(&list->sync, NULL);
	return list;
}

void list_destroy(list_t* list) {
	node_t* current = list->first;
	while(current) {
	    node_t* next = current->next;
	    pthread_mutex_destroy(&current->sync);
	    free(current);
	    current = next;
	}
	pthread_mutex_destroy(&list->sync);
	free(list);
}

void list_insert(list_t* list, const char* str) {
	node_t* new = malloc(sizeof(node_t));
	strncpy(new->value, str, MAX_LEN);
	new->next = NULL;
	pthread_mutex_init(&new->sync, NULL);

	pthread_mutex_lock(&new->sync);
	if (!list->first) list->first = new;
	else {
		new->next = list->first;
		list->first = new;
	}
	pthread_mutex_unlock(&new->sync);
}

void list_swap(node_t* a, node_t* b) {
	if (a == b) return;

	char tmp[MAX_LEN];
	strncpy(tmp, a->value, MAX_LEN);
	strncpy(a->value, b->value, MAX_LEN);
	strncpy(b->value, tmp, MAX_LEN);
}