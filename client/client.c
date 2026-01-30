#include <stdio.h>      // printf, perror
#include <stdlib.h>     // exit, malloc, free
#include <string.h>     // memset, strlen
#include <unistd.h>     // close
#include <arpa/inet.h>  // sockaddr_in, inet_addr
#include <sys/socket.h> // socket, connect
#include <pthread.h>    // pthread_create, pthread_join

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080
#define BUFFER_SIZE 4096
#define NUM_CLIENTS 5

// each thread will run this function
void* client_thread(void* arg) {
    char* message = (char*)arg;
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_sent, bytes_received, total_sent;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { perror("socket failed"); return NULL; }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect failed"); close(sock); return NULL;
    }

    for (int i = 0; i < 3; i++) { // send the message 3 times
        size_t msg_len = strlen(message);
        total_sent = 0;
        while (total_sent < msg_len) {
            bytes_sent = send(sock, message + total_sent, msg_len - total_sent, 0);
            if (bytes_sent < 0) { perror("send failed"); close(sock); return NULL; }
            total_sent += bytes_sent;
        }

        bytes_received = recv(sock, buffer, BUFFER_SIZE, 0);
        if (bytes_received < 0) { perror("recv failed"); break; }
        else {
            buffer[bytes_received] = '\0';
            printf("thread %ld received: %s\n", pthread_self(), buffer);
        }
    }

    close(sock);
    return NULL;
}


int main() {
    pthread_t threads[NUM_CLIENTS];
    char* messages[NUM_CLIENTS] = {
        "hello server from client 1",
        "hello server from client 2",
        "hello server from client 3",
        "hello server from client 4",
        "hello server from client 5"
    };

    // create threads
    for (int i = 0; i < NUM_CLIENTS; i++) {
        if (pthread_create(&threads[i], NULL, client_thread, messages[i]) != 0) {
            perror("pthread_create failed");
        }
    }

    // wait for threads to finish
    for (int i = 0; i < NUM_CLIENTS; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
