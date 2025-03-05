#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <time.h>
#include <pthread.h>

typedef enum {
	LOG_DEBUG,
	LOG_INFO,
	LOG_ERR,
} LogLevel;

typedef struct {
	FILE* file;
	LogLevel level;
	pthread_mutex_t mutex;
} Logger;

Logger* createLogger(const char* filename, LogLevel level);
void logMessage(Logger* logger, LogLevel level, const char* format, ...);
void closeLogger(Logger* logger);

#endif // LOGGER_H