#include "proxy.h"

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define CACHE_MAX_SIZE 5

CacheEntry* cache = NULL;
size_t cacheSize = 0;
pthread_mutex_t cacheMutex = PTHREAD_MUTEX_INITIALIZER;

void removeOldest(void) {
  pthread_mutex_lock(&cacheMutex);

  if (!cache) {
    pthread_mutex_unlock(&cacheMutex);
    return;
  }

  CacheEntry *entry = cache, *previous = NULL;
  CacheEntry *oldest = cache, *prevOldest = NULL;

  while (entry) {
    if (entry->lastAccessTime < oldest->lastAccessTime) {
      oldest = entry;
      prevOldest = previous;
    }
    previous = entry;
    entry = entry->next;
  }

  if (prevOldest) {
    prevOldest->next = oldest->next;
  } else {
    cache = oldest->next;
  }

  free(oldest->data);
  pthread_mutex_destroy(&oldest->mutex);
  pthread_cond_destroy(&oldest->cond);
  free(oldest);

  --cacheSize;
  pthread_mutex_unlock(&cacheMutex);
}

CacheEntry* getCacheEntry(const char* url) {
  pthread_mutex_lock(&cacheMutex);

  CacheEntry* entry = cache;
  while (entry) {
    if (strcmp(entry->url, url) == 0) {
      entry->lastAccessTime = time(NULL);
      pthread_mutex_unlock(&cacheMutex);
      return entry;
    }
    entry = entry->next;
  }

  pthread_mutex_unlock(&cacheMutex);
  return NULL;
}

CacheEntry* addNewEntry(const char* url) {
  pthread_mutex_lock(&cacheMutex);

  if (cacheSize >= CACHE_MAX_SIZE) {
    removeOldest();
  }

  CacheEntry* entry = malloc(sizeof(CacheEntry));
  if (!entry) {
    pthread_mutex_unlock(&cacheMutex);
    return NULL;
  }

  snprintf(entry->url, sizeof(entry->url), "%s", url);
  entry->data = NULL;
  entry->dataSize = 0;
  entry->downloadedSize = 0;
  entry->isComplete = 0;
  entry->lastAccessTime = time(NULL);
  if (pthread_mutex_init(&entry->mutex, NULL) != 0) {
    perror("Failed to initialize mutex");
    exit(1);
  }

  pthread_condattr_t cond_attr;
  pthread_condattr_init(&cond_attr);
  pthread_cond_init(&entry->cond, &cond_attr);

  entry->next = cache;
  cache = entry;
  cacheSize++;

  pthread_mutex_unlock(&cacheMutex);
  return entry;
}

void deleteEntry(CacheEntry* entry) {
  pthread_mutex_lock(&cacheMutex);

  CacheEntry *current = cache, *previous = NULL;
  while (current) {
    if (current == entry) {
      if (previous) {
        previous->next = current->next;
      } else {
        cache = current->next;
      }

      free(current->data);
      pthread_mutex_destroy(&current->mutex);
      pthread_cond_destroy(&current->cond);
      free(current);

      cacheSize--;
      break;
    }
    previous = current;
    current = current->next;
  }

  pthread_mutex_unlock(&cacheMutex);
}

void cacheInsertData(CacheEntry* entry, const char* data, size_t length) {
  pthread_mutex_lock(&entry->mutex);

  void* newData = realloc(entry->data, entry->downloadedSize + length);
  if (!newData) {
    pthread_mutex_unlock(&entry->mutex);
    return;
  }

  entry->data = newData;
  memcpy((char*)entry->data + entry->downloadedSize, data, length);
  entry->downloadedSize += length;

  pthread_mutex_unlock(&entry->mutex);
}

void cacheMarkComplete(CacheEntry* entry) {
  pthread_mutex_lock(&entry->mutex);

  entry->isComplete = 1;
  pthread_cond_broadcast(&entry->cond);

  pthread_mutex_unlock(&entry->mutex);
}

void cacheCleanUp(void) {
  pthread_mutex_lock(&cacheMutex);

  CacheEntry* current = cache;
  while (current) {
    CacheEntry* next = current->next;
    free(current->data);
    pthread_mutex_destroy(&current->mutex);
    pthread_cond_destroy(&current->cond);
    free(current);
    current = next;
  }

  cache = NULL;
  cacheSize = 0;

  pthread_mutex_unlock(&cacheMutex);
}
