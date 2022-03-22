/*
*   Name: Thiago André Ferreira Medeiros
*/
#include <stdio.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>

#define IP_ADDR "10.247.53.97"   /* osnode16.cse.usf.edu IP address, where server must run */
//#define IP_ADDR "127.0.0.1"
#define PORT_NUM 10501
#define BUFFLEN 100
#define n 3

// Structure for thread data
struct threadDat {
    // Identify each socket
    int threadNumber;
    // Socket data
    int connectionSocket;
};

char buf[BUFFLEN];
pthread_mutex_t mutex;

// Socket handler thread
void *connection_thread(void *args) {
    struct threadDat *arguments = args;

    printf("Connection %d open\n", arguments->threadNumber);

    bool sent = false;
    while(!sent) {
        // Wait to acquire lock
        if (pthread_mutex_trylock(&mutex) == 0) {
            // Read and stores in shared memory
            int len = recv(arguments->connectionSocket, buf, BUFFLEN, 0);

            printf("Connection %d Received: %s\n", arguments->threadNumber, buf);

            sleep(2);

            printf("Connection %d sending %s\n", arguments->threadNumber, buf);

            // Send from shared memory
            send(arguments->connectionSocket, buf, len, 0);

            sent = true;

            // Free lock
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

            // Get thread data
            args->connectionSocket = connectionSocket;
            args->threadNumber = i;

            // Create a thread to handle each socket
            pthread_create(&connections[i], NULL, connection_thread, args);

            i++;
        }
    }

    // Wait for all threads to finish
    for (i = 0; i < n; i++) {
        pthread_join(connections[i], NULL);
    }

    // Close connection
    close(fd);

    return 0;
}
