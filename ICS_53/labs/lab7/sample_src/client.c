#ifndef SAMPLE_H
#define SAMPLE_H

#include "sample.h"

#endif


void run_client(char *server_addr_str, int server_port){
    int sockfd, connfd, len;
    struct sockaddr_in servaddr, cli;

    char buffer[BUFFER_SIZE];

    // socket create and varification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("socket creation failed\n");
        exit(EXIT_FAILURE);
    }
    else
        printf("Socket successfully created\n");

    bzero(&servaddr, sizeof(servaddr));

    printf("Try connecting to server @ %s on port %d\n", server_addr_str, server_port);

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(server_addr_str);
    servaddr.sin_port = htons(server_port);

    // connect the client socket to server socket
    if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) {
        printf("connection with the server failed...\n");
        exit(EXIT_FAILURE);
    }
    else
        printf("connected to the server..\n");

    sleep(1);

    bzero(buffer, BUFFER_SIZE);
    printf("Enter the message that sent to the server: ");
    int n = 0;
    while ((buffer[n++] = getchar()) != '\n' && n < BUFFER_SIZE){
        ;
    }
    write(sockfd, buffer, n);
    bzero(buffer, BUFFER_SIZE);
    read(sockfd, buffer, BUFFER_SIZE);
    printf("Message received from Server : %s", buffer);

    // close the socket
    close(sockfd);
    return;
}

int main(int argc, char *argv[]){
    int opt;

    char *server_addr_str = NULL;
    unsigned int server_port = 0;
    while ((opt = getopt(argc, argv, "h:p:")) != -1) {
        switch (opt) {
        case 'h':
            server_addr_str = strdup(optarg);
            break;
        case 'p':
            server_port = atoi(optarg);
            break;
        default: /* '?' */
            fprintf(stderr, "Client Application Usage: %s -h <server_ip> -p <server_port_number>\n",
                    argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if (server_addr_str == NULL){
        fprintf(stderr, "ERROR: Server's IP addr is not given\n");
        fprintf(stderr, "Client Application Usage: %s -h <server_ip> -p <server_port_number>\n",
                    argv[0]);
        exit(EXIT_FAILURE);
    }

    if (server_port == 0){
        fprintf(stderr, "ERROR: Server's port number for client to connect is not given\n");
        fprintf(stderr, "Client Application Usage: %s -h <server_ip> -p <server_port_number>\n",
                    argv[0]);
        exit(EXIT_FAILURE);
    }

    run_client(server_addr_str, server_port);

    return 0;
}
