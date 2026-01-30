#include <stdio.h>      // printf, perror
#include <stdlib.h>     // exit, malloc, free
#include <string.h>     // memset
#include <unistd.h>     // close
#include <arpa/inet.h>  // sockaddr_in, inet_addr
#include <sys/socket.h> // socket, bind, listen, accept
#include <pthread.h>    // pthread_create, pthread_detach, mutex

#define PORT 8080
#define BACKLOG 10

// global client counter 
int connected_clients = 0;

// Mutex to protect the client counter 
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

#define BUFFER_SIZE 4096

// thread function to handle a single client
void* handle_client(void* arg) {
    int client_fd = *(int*)arg;
    free(arg);
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received, bytes_sent, total_sent;

    // increment client counter in critical section
    pthread_mutex_lock(&clients_mutex);
    connected_clients++;
    printf("client connected. active clients: %d\n", connected_clients);
    pthread_mutex_unlock(&clients_mutex);

    while (1) {
        // receive data from client
        bytes_received = recv(client_fd, buffer, BUFFER_SIZE, 0);
        if (bytes_received < 0) {
            perror("recv failed");
            break;
        } else if (bytes_received == 0) {
            // client closed connection
            break;
        }

        // convert lowercase letters to uppercase
        for (ssize_t i = 0; i < bytes_received; i++) {
            if (buffer[i] >= 'a' && buffer[i] <= 'z') {
                buffer[i] = buffer[i] - 'a' + 'A';
            }
        }

        // send back to client (handle partial sends)
        total_sent = 0;
        while (total_sent < bytes_received) {
            bytes_sent = send(client_fd, buffer + total_sent, bytes_received - total_sent, 0);
            if (bytes_sent < 0) {
                perror("send failed");
                break;
            }
            total_sent += bytes_sent;
        }
    }

    close(client_fd);

    // decrement client counter in critical section
    pthread_mutex_lock(&clients_mutex);
    connected_clients--;
    printf("client disconnected. active clients: %d\n", connected_clients);
    pthread_mutex_unlock(&clients_mutex);

    return NULL;
}


int main() {
    int server_fd;
    struct sockaddr_in server_addr;

    // create TCP socket 
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Bind 
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Listen 
    if (listen(server_fd, BACKLOG) < 0) {
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Multithreaded server listening on 127.0.0.1:%d\n", PORT);

    while (1) {
        int* client_fd = malloc(sizeof(int));
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        pthread_t tid;

        *client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (*client_fd < 0) {
            perror("accept failed");
            free(client_fd);
            continue;
        }

        //create a new thread for the client 
        if (pthread_create(&tid, NULL, handle_client, client_fd) != 0) {
            perror("pthread_create failed");
            close(*client_fd);
            free(client_fd);
            continue;
        }

        //detach thread to reclaim resources automatically 
        pthread_detach(tid);
    }

    close(server_fd);
    return 0;
}
