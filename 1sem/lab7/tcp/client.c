#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>

#define BUFFER_SIZE 512
#define PORT 8888

void myShutdown(char *s) {
    perror(s);
    exit(1);
}

int main() {
    int mySocket;
    struct sockaddr_in server;
    char message[BUFFER_SIZE], reply[BUFFER_SIZE];

    if ((mySocket = socket(AF_INET , SOCK_STREAM , IPPROTO_TCP)) == -1) {
        myShutdown("Socket create error");
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_port = htons(PORT);

    if (connect(mySocket , (struct sockaddr *)&server , sizeof(server)) < 0) {
        close(mySocket);
        myShutdown("Error connect");
    }

    printf("Exit command: exit999\n");
    while(1) {
        printf("Print smth: ");
        scanf("%s", message);

        if (strcmp(message, "exit999") == 0) {
            break;
        }

        if(send(mySocket, message, strlen(message), 0) == -1) {
            close(mySocket);
            myShutdown("Send error");
        }

        if(recv(mySocket , reply, BUFFER_SIZE , 0) == -1) {
            close(mySocket);
            myShutdown("Receive error");
        }

        printf("Server reply : %s\n", reply);

        memset(reply, 0, BUFFER_SIZE);
        memset(message, 0, BUFFER_SIZE);
    }

    close(mySocket);
    return 0;
}