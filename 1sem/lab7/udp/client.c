#include<stdio.h>
#include<stdlib.h>
#include<arpa/inet.h>
#include<string.h>
#include<sys/socket.h>
#include <unistd.h>

#define SERVER "127.0.0.1"
#define BUFFER_SIZE 512
#define PORT 8888

void myShutdown(char *s) {
    perror(s);
    exit(1);
}

int main() {
    struct sockaddr_in server;
    int mySocket;
    socklen_t socketLen = sizeof(server);

    char buf[BUFFER_SIZE];
    char message[BUFFER_SIZE];

    if ((mySocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        myShutdown("Socket create error");
    }

    memset((char *) &server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);

    if (inet_aton(SERVER, &server.sin_addr) == 0) {
        myShutdown("anton error");
    }

    printf("Exit command: exit999\n");
    while(1) {
        printf("Print smth: ");
        scanf("%s", message);

        if (strcmp(message, "exit999") == 0) {
            break;
        }

        if (sendto(mySocket, message, strlen(message),
                   0, (struct sockaddr *) &server, socketLen) < 0){
            myShutdown("Error while sending");
        }

        memset(buf, 0, BUFFER_SIZE);

        if (recvfrom(mySocket, buf, BUFFER_SIZE, 0,
                     (struct sockaddr *) &server, &socketLen) < 0) {
            myShutdown("Error while receive");
        }

        printf("Server reply : %s\n",buf);
    }

    close(mySocket);
    return 0;
}