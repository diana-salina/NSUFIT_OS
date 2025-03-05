#include "proxy.h"
#include "logger.h"

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <sys/poll.h>
#include <pthread.h>

#define MAX_SERVER_QUEUE_SIZE 2
#define HOST_SIZE 256
#define PATH_SIZE 1024
#define HTTP_PORT 80

typedef struct {
    int clientSocket;
    Logger* logger;
} ClientThreadRoutineArgs;

Logger* mainLog;

void* clientThreadRoutine(void* args) {
  if (!args) return NULL;

  ClientThreadRoutineArgs* routineArgs = (ClientThreadRoutineArgs*)args;
  int clientSocket = routineArgs->clientSocket;
  Logger* logger = routineArgs->logger;

  handleRequest(clientSocket, logger);

  free(routineArgs);
  close(clientSocket);
  return NULL;
}

void startProxy(int port, Logger* logger) {
  int serverSocket;
  struct sockaddr_in serverAddr;

  serverSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (serverSocket < 0) {
    perror("[ERROR] startProxy: Cannot create socket");
    exit(EXIT_FAILURE);
  }

  serverAddr.sin_family = AF_INET;
  serverAddr.sin_addr.s_addr = INADDR_ANY;
  serverAddr.sin_port = htons(port);

  int opt = 1;
  setsockopt(serverSocket, SOL_SOCKET,  SO_REUSEADDR, &opt, sizeof(opt));

  if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
    perror("[ERROR] startProxy: Cannot bind socket");
    close(serverSocket);
    exit(EXIT_FAILURE);
  }

  if (listen(serverSocket, MAX_SERVER_QUEUE_SIZE) < 0) {
    perror("[ERROR] startProxy: Listening error");
    close(serverSocket);
    exit(EXIT_FAILURE);
  }

  logMessage(logger, LOG_INFO, "Proxy server started on port %d", port);

  while (1) {
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);

    if (clientSocket < 0) {
      logMessage(logger, LOG_ERR, "Error accepting connection");
      continue;
    }

    ClientThreadRoutineArgs* args = malloc(sizeof(ClientThreadRoutineArgs));
    if (!args) {
      logMessage(logger, LOG_ERR, "Memory allocation error for client args");
      close(clientSocket);
      continue;
    }

    args->clientSocket = clientSocket;
    args->logger = logger;

    pthread_t clientThreadId;
    if (pthread_create(&clientThreadId, NULL, clientThreadRoutine, args) != 0) {
      logMessage(logger, LOG_ERR, "Error creating client thread");
      free(args);
      close(clientSocket);
      continue;
    }

    pthread_detach(clientThreadId);
  }
}

int parseURL(const char* url, char* host, char* path, int* port) {
  if (!url || !host || !path || !port) return -1;

  *port = HTTP_PORT;
  char* workUrl = malloc(URL_MAX_LEN);
  strcpy(workUrl, url);
  if (!workUrl) return -1;

  if (strchr(workUrl, '@')) {
  	free(workUrl);
  	return -1;
  } else if (sscanf(workUrl, "http://%255[^:/]:%d/%1023s", host, port, path) == 3) {
    // URL with port and path
  } else if (sscanf(workUrl, "http://%255[^/]/%1023s", host, path) == 2) {
    // URL with path, no port
  } else if (sscanf(workUrl, "http://%255[^:/]:%d", host, port) == 2) {
    // URL with port, no path
  } else if (sscanf(workUrl, "http://%255[^/]", host) == 1) {
    // Only host
    strcpy(path, "/");
  } else {
    free(workUrl);
    return -1;
  }

  free(workUrl);
  return 0;
}

void handleRequest(int clientSocket, Logger* logger) {
  char buffer[BUFFER_SIZE], method[16], url[URL_MAX_LEN], protocol[16];
  mainLog = logger;

  if (recv(clientSocket, buffer, sizeof(buffer), 0) <= 0) return;
  if (sscanf(buffer, "%15s %2047s %15s", method, url, protocol) != 3) {
  	logMessage(mainLog, LOG_ERR, "Method, URL or protocol is too long");
  	return;
  }

  if (strcmp(method, "GET") != 0) {
    logMessage(logger, LOG_ERR, "405 Method Not Allowed");
    return;
  }


  logMessage(mainLog, LOG_INFO, "New connection: %s", url);
  CacheEntry* entry = getCacheEntry(url);
  if (!entry) {
    entry = addNewEntry(url);

    pthread_create(&entry->dowloadThread, NULL, downloadData, entry);
    pthread_detach(entry->dowloadThread);
  } else {
    logMessage(logger, LOG_INFO, "Cache hit for URL: %s", url);
  }

  pthread_mutex_lock(&entry->mutex);
  while (!entry->isComplete) {
    pthread_cond_wait(&entry->cond, &entry->mutex);
  }
  if (entry->downloadedSize > 0) {
    send(clientSocket, entry->data, entry->downloadedSize, 0);
  }

  pthread_mutex_unlock(&entry->mutex);
}

void* downloadData(void* args) {
  if (!args) return NULL;

  logMessage(mainLog, LOG_INFO, "Downloading");
  CacheEntry* entry = (CacheEntry*)args;
  char host[HOST_SIZE], path[PATH_SIZE];
  int port;

  if (parseURL(entry->url, host, path, &port) != 0) {
    logMessage(mainLog, LOG_ERR, "Error parsing URL");
    deleteEntry(entry);
    return NULL;
  }

  struct hostent* server = gethostbyname(host);
  if (!server) {
    logMessage(mainLog, LOG_ERR, "Cannot resolve host: %s", host);
    return NULL;
  }

  int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (serverSocket < 0) {
    logMessage(mainLog, LOG_ERR, "Error creating socket for host: %s", host);
    return NULL;
  }

  struct sockaddr_in serverAddr = {
          .sin_family = AF_INET,
          .sin_port = htons(port),
  };
  memcpy(&serverAddr.sin_addr.s_addr, server->h_addr_list[0], server->h_length);

  if (connect(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
    logMessage(mainLog, LOG_ERR, "Error connecting to host: %s:%d", host, port);
    close(serverSocket);
    return NULL;
  }

  char request[BUFFER_SIZE];
  snprintf(request, sizeof(request), "GET /%s HTTP/1.0\r\nHost: %s\r\n\r\n", path, host);
  send(serverSocket, request, strlen(request), 0);

  char buffer[BUFFER_SIZE];
  char headerBuffer[BUFFER_SIZE];
  ssize_t bytesReceived;
  size_t headerLength = 0;
  int isChecked = 0;
  while ((bytesReceived = recv(serverSocket, buffer, sizeof(buffer), 0)) > 0) {
    if (!isChecked) {
            size_t copyLength = sizeof(headerBuffer) - headerLength - 1;
            if (bytesReceived < copyLength) copyLength = bytesReceived;

            memcpy(headerBuffer + headerLength, buffer, copyLength);
            headerLength += copyLength;

            char* headersEnd = strstr(headerBuffer, "\r\n\r\n");
            if (headersEnd) {
                //isChecked = 1;

                if (!(strstr(headerBuffer, "HTTP/1.0 200 OK") || strstr(headerBuffer, "HTTP/1.1 200 OK"))) {
                    logMessage(mainLog, LOG_ERR, "Received not 200 status");

                    const char* errorResponse =
                        "HTTP/1.1 502 Bad Gateway\r\n"
                        "Content-Type: text/plain\r\n"
                        "Content-Length: 43\r\n\r\n"
                        "Error: Received non-200 response from server";
                    cacheInsertData(entry, errorResponse, sizeof(errorResponse));
                    break;
                }

                size_t headersSize = headersEnd + 4 - headerBuffer;
                size_t remainingBytes = headerLength - headersSize;
                if (remainingBytes > 0) {
                    cacheInsertData(entry, headerBuffer + headersSize, remainingBytes);
                }

                size_t dataOffset = bytesReceived - copyLength;
                if (dataOffset > 0) {
                    cacheInsertData(entry, buffer + copyLength, dataOffset);
                }
                continue;
            }

            if (headerLength >= sizeof(headerBuffer) - 1) {
                logMessage(mainLog, LOG_ERR, "Header too large");

                const char* errorResponse =
                    "HTTP/1.1 502 Bad Gateway\r\n"
                    "Content-Type: text/plain\r\n"
                    "Content-Length: 35\r\n\r\n"
                    "Error: Response headers too large";
                cacheInsertData(entry, errorResponse, sizeof(errorResponse));
                break;
            }
            isChecked = 1;
        } else {
            cacheInsertData(entry, buffer, bytesReceived);
        }

  }
  cacheMarkComplete(entry);
  close(serverSocket);
  return NULL;
}
