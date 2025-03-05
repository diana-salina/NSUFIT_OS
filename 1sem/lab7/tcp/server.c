#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<pthread.h>

#define BUFFER_SIZE 512
#define PORT 8888

int isFinished = 0;
struct sockaddr_in client;

void myShutdown(char *s) {
    perror(s);
    exit(1);
}

void *inputHandler(){
    char c = 0;
    while(c != 'q') {
        scanf("%c", &c);
    }
    isFinished = 1;
    myShutdown("Exit");
    return 0;
}

void *connectionHandler(void * s) {
    printf("Client %d connected\n", client.sin_port);

    int *clientSocket = (int *) s;
    char clientReply[BUFFER_SIZE];

    memset(clientReply, 0, BUFFER_SIZE);

    size_t receiveLen;
    while((receiveLen = recv(*clientSocket, clientReply, BUFFER_SIZE, 0)) > 0
                            && (isFinished == 0)) {
        printf("Recieved from %d: %s\n", client.sin_port, clientReply);
        int w = write(*clientSocket, clientReply , strlen(clientReply));
        if (w <= 0) {
            printf("Error write");
            break;
        }
        memset(clientReply, 0, BUFFER_SIZE);
    }

    if (receiveLen <= 0) {
        printf("Client %d disconnected\n", client.sin_port);
    }
    close(*clientSocket);
    return 0;
}

int main() {
    int mySocket, clientSocket;
    int *newClient;
    struct sockaddr_in server;

    if ((mySocket = socket(AF_INET , SOCK_STREAM , IPPROTO_TCP)) == -1) {
        myShutdown("Socket create error");
    }

    memset((char *) &server, 0, sizeof(server));

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htons(INADDR_ANY);
    server.sin_port = htons(PORT);

    if (bind(mySocket, (struct sockaddr *)&server , sizeof(server)) == -1) {
        close(mySocket);
        myShutdown("Bind error");
    }

    if(listen(mySocket , 3) < 0){
        myShutdown("Listen error");
    }

    pthread_t thrInput;

    int e;
    e = pthread_create(&thrInput, NULL,
                   inputHandler, NULL);
    if(e){
        myShutdown("Thread create error");
    }

    printf("Waiting for connections...\n");
    socklen_t socketLen = sizeof(struct sockaddr_in);
    while((clientSocket = accept(mySocket,
                                 (struct sockaddr *)&client, &socketLen))) {
        if(clientSocket == -1){
            myShutdown("Accept error");
        }
        printf("Connection accepted\n");

        newClient = malloc(sizeof(clientSocket));
        *newClient = clientSocket;

        pthread_t thr;
        e = pthread_create(&thr, NULL,
                       connectionHandler, newClient);
        if(e){
            myShutdown("Thread create error");
        }
        printf("Handler assigned\n");
    }

    close(mySocket);
    if (clientSocket < 0) {
        myShutdown("Error client connection");
    }
    return 0;
}