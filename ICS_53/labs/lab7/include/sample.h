#include <getopt.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define SA struct sockaddr

void run_server(int server_port);
void run_client(char *server_addr_str, int server_port);