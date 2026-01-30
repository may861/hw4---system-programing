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

// thread function to handle a single client 
void* handle_client(void* arg) {
    int client_fd = *(int*)arg;
    free(arg);

    //enter critical section 
    pthread_mutex_lock(&clients_mutex);
    connected_clients++;
    printf("Client connected. Active clients: %d\n", connected_clients);
    pthread_mutex_unlock(&clients_mutex);
    //exit critical section 

    //  close 
    close(client_fd);

    //enter critical section 
    pthread_mutex_lock(&clients_mutex);
    connected_clients--;
    printf("Client disconnected. Active clients: %d\n", connected_clients);
    pthread_mutex_unlock(&clients_mutex);
    //exit critical section 

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
