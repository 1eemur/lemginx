/* main.c - Entry point for the web server */
#include "common.h"
#include "logging.h"
#include "client_handler.h"

int main() {
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);
    int server_fd, max_connections = 10;
    struct sockaddr_in server_addr;

    message_len = snprintf(log_message, sizeof(log_message), 
                          "lemginx starting");
    handle_log(log_message, DEBUG);

    // Create the server socket
    if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }
    
    // AF_INET, use IPv4 (AF_INET6 for IPv6)
    server_addr.sin_family = AF_INET;
    // INADDR_ANY, tells the server to accept connections from any network interface
    server_addr.sin_addr.s_addr = INADDR_ANY;
    // Convert hostbyte order from Least Significant Bit first to network byte order, which is Most Signficiant byte first
    server_addr.sin_port = htons(PORT);

    // Binds the socket to a port defined above, causing the socket to listen for any clients trying connect on that port
    message_len = snprintf(log_message, sizeof(log_message), 
                          "Binding socket to port %d", PORT);
    handle_log(log_message, DEBUG);
    if(bind(server_fd, 
            (struct sockaddr*)&server_addr,
            sizeof(server_addr)) < 0){
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    message_len = snprintf(log_message, sizeof(log_message), 
                          "Listening for connections on port %d", PORT);
    handle_log(log_message, INFO);
    if(listen(server_fd, max_connections) < 0){
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    while(1){
        // Define client info
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int *client_fd = malloc(sizeof(int));

        // Accept client connections
        if((*client_fd = accept(server_fd,
                    (struct sockaddr *)&client_addr, 
                    &client_addr_len)) < 0){
            perror("Client accept failed");
            continue;
        }

        // Create new thread to handle client request
        pthread_t thread_id;
        pthread_create(&thread_id, NULL, handle_client, (void *)client_fd);
        pthread_detach(thread_id);
    }
    return 0;
}