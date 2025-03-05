#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include <unistd.h>
#include <pthread.h>

#define BUFFER_SIZE 512
#define PORT 8888

int isFinished = 0;

void myShutdown(char *s) {
    perror(s);
    exit(1);
}

void * messageHandler(void * sock){
    struct sockaddr_in client;
    int mySocket = *(int*) sock;

    size_t receiveLen;
    socklen_t socketLen = sizeof(struct sockaddr_in);

    char buf[BUFFER_SIZE];

    fflush(stdout);
    while (isFinished != 1) {
        if ((receiveLen = recvfrom(mySocket, buf, BUFFER_SIZE,
                                   0, (struct sockaddr *) &client, &socketLen)) == -1) {
            myShutdown("Receive error");
        }

        printf("Received from %d: %s\n", ntohs(client.sin_port), buf);

        if (sendto(mySocket, buf, receiveLen, 0,
                   (struct sockaddr *) &client, socketLen) == -1) {
            myShutdown("Send error");
        }
    }
    free(sock);
    pthread_exit(NULL);
}

int main() {
    struct sockaddr_in server;

    int mySocket;

    if ((mySocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        myShutdown("Socket create error");
    }

    memset((char *) &server, 0, sizeof(server));

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(mySocket, (struct sockaddr*)&server,
             sizeof(server)) == -1) {
        close(mySocket);
        myShutdown("Bind socket error");
    }

    pthread_t thr;
    int *socketPointer;
    socketPointer = malloc(1);
    *socketPointer = mySocket;
    int ex = pthread_create(&thr, NULL, messageHandler, socketPointer);
    if (ex) {
        myShutdown("Thread create error");
    }
    char c = 0;
    while(c != 'q') {
        scanf("%c", &c);
    }

    isFinished = 1;
    close(mySocket);
    return 0;
}