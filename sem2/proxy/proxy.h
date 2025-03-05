#ifndef PROXY_H
#define PROXY_H

#include "logger.h"
#include <pthread.h>

#define PORT 8080
#define CACHE_SIZE (1024*1024*60) //60Mb
#define BUFFER_SIZE 16384 //16kb
#define URL_MAX_LEN 2048

typedef struct CacheEntry {
	char url[URL_MAX_LEN];
	char* data;
	size_t dataSize;
	size_t downloadedSize;
	int isComplete;
	time_t lastAccessTime;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	pthread_t dowloadThread;
	struct CacheEntry* next;
} CacheEntry;

void startProxy(int port, Logger* logger);
void handleRequest(int clientSocket, Logger* logger);
void* downloadData(void* args);

CacheEntry* getCacheEntry(const char* url);
CacheEntry* addNewEntry(const char* url);
void deleteEntry(CacheEntry* entry);
void cacheCleanUp(void);
void cacheInsertData(CacheEntry* entry, const char* data, size_t length);
void cacheMarkComplete(CacheEntry* entry);

#endif // PROXY_H