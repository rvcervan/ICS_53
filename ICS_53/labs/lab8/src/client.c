#ifndef SAMPLE_H
#define SAMPLE_H

#include "sample.h"

#endif

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

int client_id = 0;
int sockfd;
const char USEAGE_STR[] = "Client Application Usage: %s -i <client_id> -h <server_ip_or_hostname> -p <server_port_number>\n";
const char exit_str[] = "exit";

void sigint_handler(int sig) 
{
    printf("SIGINT detected, sending exit string to server\n");
    write(sockfd, exit_str, sizeof(exit_str));
    exit(0);
}

void run_client(char *server_addr_str, int server_port){
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
    struct hostent *he;
    if ((he = gethostbyname(server_addr_str)) == NULL) {  // get the host info
        printf("Error: gethostbyname, cannot get server's address by its hostname");
        exit(EXIT_FAILURE);
    }
    char *addr = inet_ntoa(*(((struct in_addr **)he->h_addr_list)[0]));
    printf("IP address of server is: %s\n", addr);
    servaddr.sin_addr.s_addr = inet_addr(addr);
    servaddr.sin_port = htons(server_port);

    // connect the client socket to server socket
    if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) {
        printf("connection with the server failed...\n");
        exit(EXIT_FAILURE);
    }
    else
        printf("connected to the server..\n");

    if (signal(SIGINT, sigint_handler) == SIG_ERR) 
	    printf("signal handler processing error\n");
    sleep(1);

    int msg_count = 1;
    while(1){
        sleep(1);
        bzero(buffer, sizeof(buffer));
        snprintf(buffer, sizeof(buffer), "client %d: message #%d", client_id, msg_count);
        int msg_len = strlen(buffer);
        write(sockfd, buffer, msg_len);
        printf("Sent message \"%s\" to server\n", buffer);
        msg_count ++;

        if(strncmp(exit_str,buffer,sizeof(exit_str))==0){
            break;
        }

        bzero(buffer, sizeof(buffer));
        read(sockfd, buffer, sizeof(buffer));
        printf("Message received from Server : %s\n\n", buffer);
    }

    // close the socket
    close(sockfd);
    return;
}

int main(int argc, char *argv[])
{
    int opt;

    char *server_addr_str = NULL;
    unsigned int server_port = 0;
    while ((opt = getopt(argc, argv, "h:p:i:")) != -1) {
        switch (opt) {
        case 'i':
            client_id = atoi(optarg);
            break;
        case 'h':
            server_addr_str = strdup(optarg);
            break;
        case 'p':
            server_port = atoi(optarg);
            break;
        default: /* '?' */
            fprintf(stderr, USEAGE_STR,
                    argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if (server_addr_str == NULL){
        fprintf(stderr, "ERROR: Server's IP addr or hostname  is not given\n");
        fprintf(stderr, USEAGE_STR,
                    argv[0]);
        exit(EXIT_FAILURE);
    }

    if (server_port == 0){
        fprintf(stderr, "ERROR: Server's port number for client to connect is not given\n");
        fprintf(stderr, USEAGE_STR,
                    argv[0]);
        exit(EXIT_FAILURE);
    }

    if (client_id == 0){
        fprintf(stderr, "ERROR: Client ID is not given\n");
        fprintf(stderr, USEAGE_STR,
                    argv[0]);
        exit(EXIT_FAILURE);
    }

    run_client(server_addr_str, server_port);

    return 0;
}
