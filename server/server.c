#include <stdio.h>      // printf, perror
#include <stdlib.h>     // exit
#include <string.h>     // memset
#include <unistd.h>     // close
#include <arpa/inet.h>  // sockaddr_in, inet_addr
#include <sys/socket.h> // socket, bind, listen, accept

#define PORT 8080
#define BACKLOG 10

int main() {
    int server_fd;
    struct sockaddr_in server_addr;

    // create a TCP socket 
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    //zero out the server address structure 
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    //bind socket to IP and port 
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // start listening for incoming connections 
    if (listen(server_fd, BACKLOG) < 0) {
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on 127.0.0.1:%d\n", PORT);

    // main server loop 
    while (1) {
        int client_fd;
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        // accept an incoming connection
        client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) {
            perror("accept failed");
            continue;
        }

        printf("Client connected\n");

        // close the connection
        close(client_fd);
        printf("Client disconnected\n");
    }

    close(server_fd);
    return 0;
}
