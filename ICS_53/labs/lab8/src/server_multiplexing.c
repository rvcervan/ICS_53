#ifndef SAMPLE_H
#define SAMPLE_H

#include "sample.h"

#endif

const char exit_str[] = "exit";
int total_num_msg = 0;

char buffer[BUFFER_SIZE];

fd_set read_fds; // the fd set used in select() function (will be updated in select() function)
fd_set all_read_fds; // the fd set which includes all fds that we would like to check if they are ready to read
int all_client_fds[FD_SETSIZE]; // an array that keeps the fd of all connected clients
int max_fd; // maximum fd

void add_client(int client_fd);
void remove_client(int client_fd);

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
    int listen_fd = server_init(server_port); // Initiate server and start listening on specified port
    int client_fd;
    struct sockaddr_in client_addr;
    int client_addr_len = sizeof(client_addr);

    FD_ZERO(&all_read_fds); //
    FD_SET(listen_fd, &all_read_fds); // add server's listen socket fd to "all" fd set

    max_fd = listen_fd;

    int i;
    for(i=0; i < FD_SETSIZE; i++){
        all_client_fds[i] = -1;
    }

    while(1){
        FD_ZERO(&read_fds);
        read_fds = all_read_fds;
        max_fd = (max_fd > listen_fd)?max_fd:listen_fd;
        printf("maxfd=%d, listenfd=%d\n",max_fd,listen_fd);
    
        // Multiplex on the client socket file descriptors 
        select(max_fd+1, &read_fds, NULL, NULL, NULL);

       /* If listening descriptor ready, add new client to "all" fd set */
	    if (FD_ISSET(listen_fd, &read_fds)) {
            client_fd = accept(listen_fd, (SA*)&client_addr, &client_addr_len);
            if (client_fd < 0) {
                printf("server acccept failed\n");
                exit(EXIT_FAILURE);
            }
            else{
                printf("Client connetion accepted\n");
            }
            add_client(client_fd);
	    }
        
        /* If any client fd is ready, read and send the message */
        for(i=0; i < FD_SETSIZE; i++){
            if(all_client_fds[i] > 0 && FD_ISSET(all_client_fds[i], &read_fds)){
                bzero(buffer, BUFFER_SIZE);
                int received_size = read(all_client_fds[i], buffer, sizeof(buffer));
                if(received_size < 0){
                    printf("Receiving failed\n");
                    continue;
                }else if(received_size == 0){
                    continue;
                }
                total_num_msg ++;
                if(strncmp(exit_str, buffer, sizeof(exit_str))==0){
                    printf("Client exit\n");
                    printf("Close current client connection\n");
                    remove_client(all_client_fds[i]);
                    continue;
                }

                // print buffer which contains the client contents
                printf("Receive message from client: %s\n", buffer);
                printf("Total number of received messages: %d\n", total_num_msg);

                // and send that buffer to client
                int ret = write(all_client_fds[i], buffer, received_size);
                if (ret < 0){
                    printf("Sending failed\n");
                    continue;
                }
                printf("Send the message back to client: %s\n\n", buffer);
            }
        }
    }
    close(listen_fd);
    return;
}

void add_client(int client_fd){
    int i;
    FD_SET(client_fd, &all_read_fds);
    for(i=0; i < FD_SETSIZE; i++){
        if (all_client_fds[i] == -1){
            all_client_fds[i] = client_fd;
            break;
        }
    }
    if (max_fd < client_fd){
        max_fd = client_fd;
    }
}

void remove_client(int client_fd){
    int i;
    close(client_fd);
    FD_CLR(client_fd, &all_read_fds);
    max_fd = 0;
    for(i=0; i < FD_SETSIZE; i++){
        if (all_client_fds[i] == client_fd){
            all_client_fds[i] = -1;
            continue;
        }
        if (all_client_fds[i] > max_fd){
            max_fd = all_client_fds[i];
        }
    }
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
