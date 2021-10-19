#ifndef SAMPLE_H
#define SAMPLE_H

#include "sample.h"
#include <signal.h>
#endif

const char exit_str[] = "exit";
int total_num_msg = 0;
int listen_fd;

char buffer[BUFFER_SIZE];

void sigint_handler(int sig)
{
    write(0,"shutting down server\n",strlen("shutting down server\n"));
    close(listen_fd);
    exit(0);
}

int server_init(int server_port){
    int sockfd;
    struct sockaddr_in servaddr;

    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("socket creation failed...\n");
        exit(EXIT_FAILURE);
    }
    else
        printf("Socket successfully created\n");

    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(server_port);

    int opt = 1;
    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, (char *)&opt, sizeof(opt))<0)   {
        perror("setsockopt");exit(EXIT_FAILURE);
    }

    // Binding newly created socket to given IP and verification
    if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
        printf("socket bind failed\n");
        exit(EXIT_FAILURE);
    }
    else
        printf("Socket successfully binded\n");

    // Now server is ready to listen and verification
    if ((listen(sockfd, 1)) != 0) {
        printf("Listen failed\n");
        exit(EXIT_FAILURE);
    }
    else
        printf("Server listening on port: %d.. Waiting for connection\n", server_port);

    return sockfd;
}


void run_server(int server_port){
    listen_fd = server_init(server_port); // Initiate server and start listening on specified port
    int client_fd;
    struct sockaddr_in client_addr;
    int client_addr_len = sizeof(client_addr);

    if(signal(SIGINT, sigint_handler) == SIG_ERR)
	printf("signal handler processing error\n");

    while(1){
        // Accept the connection from client
        printf("Parent Process: Wait for new client connection\n");
        client_fd = accept(listen_fd, (SA*)&client_addr, &client_addr_len);
        if (client_fd < 0) {
            printf("server acccept failed\n");
            exit(EXIT_FAILURE);
        }
        else{
            pid_t pid = fork();
            if (pid == 0){
		        //Child process (per client)
                while(1){
                    bzero(buffer, BUFFER_SIZE);
                    int received_size = read(client_fd, buffer, sizeof(buffer));
                    if(received_size < 0){
                        printf("Receiving failed\n");
                        break;
                    }else if(received_size == 0){
                        continue;
                    }
                    total_num_msg ++;
                    if(strncmp(exit_str, buffer, sizeof(exit_str))==0){
                        printf("Client exit\n");
                        break;
                    }

                    // print buffer which contains the client contents
                    printf("Receive message from client: %s\n", buffer);
                    printf("Total number of received messages: %d\n", total_num_msg);

                    // and send that buffer to client
                    int ret = write(client_fd, buffer, received_size);
                    if (ret < 0){
                        printf("Sending failed\n");
                        break;
                    }
                    printf("Send the message back to client: %s\n\n", buffer);
                }
                // Close the socket at the end
                printf("Close current client connection\n");
                close(client_fd);
		        exit(0);
            }
            else{
                //Parent Process
                continue;
            }
        }
    }
    close(listen_fd);
    return;
}

int main(int argc, char* argv[])
{
    int opt;

    unsigned int port = 0;
    while ((opt = getopt(argc, argv, "p:")) != -1) {
        switch (opt) {
        case 'p':
            port = atoi(optarg);
            break;
        default: /* '?' */
            fprintf(stderr, "Server Application Usage: %s -p <port_number>\n",
                    argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if (port == 0){
        fprintf(stderr, "ERROR: Port number for server to listen is not given\n");
        fprintf(stderr, "Server Application Usage: %s -p <port_number>\n",
                    argv[0]);
        exit(EXIT_FAILURE);
    }

    run_server(port);

    return 0;
}
