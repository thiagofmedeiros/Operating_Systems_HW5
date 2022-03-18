#include <stdio.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>

//#define IP_ADDR "10.247.53.97"   /* osnode16.cse.usf.edu IP address, where server must run */
#define IP_ADDR "127.0.0.1"
#define PORT_NUM 10501
#define BUFFLEN 100
#define n 3

struct threadDat {
    int threadNumber;
    int connectionSocket;
};

char buf[BUFFLEN];
pthread_mutex_t mutex;

void *connection_thread(void *args) {
    struct threadDat *arguments = args;

    printf("Connection %d open\n", arguments->threadNumber);

    bool sent = false;
    while(!sent) {
        if (pthread_mutex_trylock(&mutex) == 0) {
            int len = recv(arguments->connectionSocket, buf, BUFFLEN, 0);

            printf("Connection %d Received: %s\n", arguments->threadNumber, buf);

            sleep(2);

            printf("sending %s\n", buf);

            send(arguments->connectionSocket, buf, len, 0);

            sent = true;

            pthread_mutex_unlock(&mutex);
        }
    }

    return (NULL);
}

int main() {
    int connectionSocket;
    int addressLength;
    struct sockaddr_in serverAddress;
    int on = 1;
    int fd;

    pthread_t connections[n];     /* process id for threads */
    pthread_mutex_init(&mutex,NULL);

    /* serverAddress naming for the internet domain socket */
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT_NUM);
    serverAddress.sin_addr.s_addr = inet_addr(IP_ADDR);

    addressLength = sizeof (serverAddress);

    /* socket() call generates one socket for ipc */
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("Opening Stream Socket");
        exit(1);
    }

    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)))
    {
        perror("setsockopt");
        exit(1);
    }

    if (bind(fd, (struct sockaddr *)&serverAddress, addressLength) < 0) {
        perror("Bind error");
        exit(1);
    }

    if (listen(fd, 3) < 0) {
        perror("Listen error");
        exit(1);
    }

    int i = 0;
    while (i < n) {
        connectionSocket = accept(fd, (struct sockaddr *) &serverAddress,
                                  (socklen_t *) &addressLength);
        if (connectionSocket < 0) {
            perror("Accept error");
        } else {
            struct threadDat *args = malloc(sizeof *args);

            args->connectionSocket = connectionSocket;
            args->threadNumber = i;

            pthread_create(&connections[i], NULL, connection_thread, args);

            i++;
        }
    }

    for (i = 0; i < n; i++) {
        pthread_join(connections[i], NULL);
    }

    close(fd);

    return 0;
}
