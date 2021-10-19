#include "server.h"



// TODO: Implement this
void server_sigint_handler() {
    
}

void init_auctions(char* auction_filename) {

    Auctions = malloc(sizeof(List_t));
    Auctions->head = NULL;
    Auctions->length = 0;
    Auctions->comparator = AuctionIDComparator;

    char* item_name = NULL;
    int duration = 0;
    int bin_price = 0;

    char abs_path[500] = "";
    ssize_t bytesread = 0;
    char* line = NULL;
    size_t line_len = 0;

    strcat(abs_path, auction_filename);

    FILE* auction_fd = fopen(abs_path, "r");
    int i = 0;
    while ((bytesread = getline(&line, &line_len, auction_fd)) != -1) {
        if (i == 0) {
            item_name = malloc(sizeof(char)*line_len);
            strcpy(item_name, line);
            item_name[line_len-2] = '\0';
        }
        else if (i == 1) {
            duration = atoi(line);
        }
        else if (i == 2) {
            bin_price = atoi(line);
        }
        else if (i == 3) {
            Auction_t* new_auction = create_auction_node(item_name, duration, bin_price);
            
            // Lock and increment aid, unlock
            sem_post(&aid_lock);
            aid++;
            sem_wait(&aid_lock);

            // Lock, insert, unlock
            sem_post(&auctions_write_lock);
            insertInOrder(Auctions, new_auction);
            sem_wait(&auctions_write_lock);

            // reset for next loop
            i = 0;
            continue;
        }

        i++;
    }
    fclose(auction_fd);
    free(line);
}

int server_init(int server_port) {
    int sockfd;
    struct sockaddr_in servaddr;

    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        fprintf(stderr, "socket creation failed...\n");
        exit(EXIT_FAILURE);
    }

    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(server_port);

    int opt = 1;
    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, (char *)&opt, sizeof(opt))<0) {
        perror("setsockopt error");
        exit(EXIT_FAILURE);
    }

    // Binding newly created socket to given IP and verification
    if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
        fprintf(stderr, "socket bind failed\n");
        exit(EXIT_FAILURE);
    }

    // Now server is ready to listen and verification
    if ((listen(sockfd, 1)) != 0) {
        fprintf(stderr, "Listen failed\n");
        exit(EXIT_FAILURE);
    }

    return sockfd;
}

void run_server(int server_port, int job_thread_count) {

    // Init aid
    aid = 1;
  
    // Initiate server and start listening on specified port
    int listen_fd = server_init(server_port);

    // TODO: Create N detached job (red) threads
    sbuf_init(&Job_Queue, job_thread_count);
    pthread_t tid;
    int i = 0;
    int retcode;
    for (i=0; i<job_thread_count; i++) {
        if ((retcode = pthread_create(&tid, NULL, Red_Thread, NULL)) != 0) {
            fprintf(stderr, "pthread_create Red_Thread error: %s\n", strerror(retcode));
            exit(0);
        }
    }

    // TODO: Create tick (yellow) thread


   
    // TODO: Init all shared resources (purple)
    Users = malloc(sizeof(List_t));
    Users->head = NULL;
    Users->length = 0;
    //Init the semaphores
    sem_init(&users_mutex, 0, 1);
    sem_init(&users_write_lock, 0, 1);
    sem_init(&auctions_mutex, 0, 1);
    sem_init(&auctions_write_lock, 0, 1);
    

    petr_header login_req_header;
    login_req_header.msg_len = 0;
    login_req_header.msg_type = 0;


    char* login_req = NULL;
    char* username = NULL;
    char* password = NULL;

    //int client_fd;//removed int*
    socklen_t client_addr_len;
    struct sockaddr_storage client_addr;
    client_addr_len = sizeof(client_addr);//added this

    while (1) {
        
        // Wait and Accept the connection from client
        printf("Wait for new client connection\n");

        int* client_fd = malloc(sizeof(int));//added int*
        //client_addr_len = sizeof(struct sockaddr_storage);commented out this
        if ((*client_fd = accept(listen_fd, (SA*)&client_addr, &client_addr_len)) < 0) {
            fprintf(stderr, "server acccept failed\n");
            exit(EXIT_FAILURE);
        }

        printf("Client connetion accepted\n");
        printf("client_fd = %d\n", *client_fd);
        
        int read_result;
        // TODO: LAB QUESTION: ERROR in rd_msgheader

        read_result = rd_msgheader(*client_fd, &login_req_header);
        printf("read_result = %d\n", read_result);
        
        login_req = malloc((login_req_header.msg_len));
        read(*client_fd, login_req, (login_req_header.msg_len));
        printf("login_req = %s\n", login_req); 
        //Parse_Login(username, password, login_req, login_req_header.msg_len);

        username = strtok(login_req, "\r\n");
        password = strtok(NULL, "\r\n");
 
        printf("username=%s password=%s\n", username, password);


        // ProcessLogin
        // Lock for reading Users
        sem_wait(&users_mutex);
        users_readcnt++;
        if (users_readcnt == 1) {
            sem_wait(&users_write_lock);
        }
        sem_post(&users_mutex);

        node_t* cur_node = Users->head;
        User_t* cur_val;
        int processedLogin = 0;
        
        User_t* new_account = NULL;
        int login_success = 0;
        printf("going into the while\n");
        while (cur_node != NULL) {
            cur_val = (User_t*)(cur_node->value);

            // error if username exists, isOnline, correct/wrong password
            if ((strcmp(cur_val->username, username) == 0) && (cur_val->isOnline)) {
                printf("Error1: EUSRLGDIN\n");
                // send EUSRLGDIN
                login_req_header.msg_len = 0;
                login_req_header.msg_type = EUSRLGDIN;
                wr_msg(cur_val->fd, &login_req_header, NULL);
                processedLogin = 1;

                sem_wait(&users_mutex);
                users_readcnt--;
                if (users_readcnt == 0) {
                    sem_post(&users_write_lock);
                }
                sem_post(&users_mutex);
                //free username and password
                break;
            }

            // error if username exists, !isOnline, wrong password
            else if ((strcmp(cur_val->username, username) == 0) && !(cur_val->isOnline) && (strcmp(cur_val->password, password) != 0)) {
                // send EWRNGPWD
                printf("Error2: EWRNGPWD\n");
                login_req_header.msg_len = 0;
                login_req_header.msg_type = EWRNGPWD;
                wr_msg(cur_val->fd, &login_req_header, NULL);
                processedLogin = 1;

                sem_wait(&users_mutex);
                users_readcnt--;
                if (users_readcnt == 0) {
                    sem_post(&users_write_lock);
                }
                sem_post(&users_mutex);
                break;
            }
            
            else { // OK cases, // Writer
                printf("OK cases\n");
                sem_wait(&users_mutex);
                users_readcnt--;
                if (users_readcnt == 0) {
                    sem_post(&users_write_lock);
                }
                sem_post(&users_mutex);
                // Done reading

                // Lock for writing to Users
                sem_wait(&users_write_lock);

                if ((strcmp(cur_val->username, username) == 0)) {
                    printf("OK: username matches and account exists\n");
                    // handle valid, send OK
                    cur_val->isOnline = 1;
                    
                    login_req_header.msg_len = 0;
                    login_req_header.msg_type = OK;
                    cur_val->fd = *client_fd;
                    wr_msg(cur_val->fd, &login_req_header, NULL);
                    login_success = 1;
                    processedLogin = 1;
                }
                sem_post(&users_write_lock);    // Unlock write

                //login_success = 1;
                //processedLogin = 1;
                printf("ok break out\n");
                break;
            }

            cur_node = cur_node->next;
        }
        printf("just out of the loop\n");
        if (!processedLogin) {  // Users list is empty
            printf("OK: new account\n");
            sem_wait(&users_mutex);
            users_readcnt--;
            if (users_readcnt == 0) {
                sem_post(&users_write_lock);
            }
            sem_post(&users_mutex);
            // Done reading

            // create new account, handle valid, send OK
            

            new_account = CreateAccount(username, password, *client_fd);
            insertFront(Users, new_account);

            // Locking for writing
            sem_wait(&users_write_lock);
            login_req_header.msg_len = 0;
            login_req_header.msg_type = OK;
            wr_msg(new_account->fd, &login_req_header, NULL);
            sem_post(&users_write_lock); // Unlock write
            //printf("Does is go here at one point?")
            login_success = 1;
        }
        printf("before login success\n");
        if (login_success) {
            GTA_t green_thread_args;
            green_thread_args.client_fd = *client_fd;
            green_thread_args.listen_fd = listen_fd;

            if (pthread_create(&tid, NULL, Green_Thread, &green_thread_args) != 0) {
                fprintf(stderr, "pthread_create Green_Thread error: %s\n", strerror(retcode));
                exit(0);
            }
        }
        free(client_fd);//because we pass them by value in everywhere I think?
        processedLogin = 0;
        login_success = 0;

    }

    close(listen_fd);
    return;
}


/*
Main Thread
*/
int main(int argc, char* argv[]) {
    int opt;
    int j_specified = 0;
    int job_thread_count;
    int t_specified = 0;
    int tick_len_seconds = 0;
    unsigned int server_port = -1;
    char* auction_filename = NULL;
    int extra_arg_idx = 0;
    extern int optind;
    extern char* optarg;

    
    while ((opt = getopt(argc, argv, "hj:t:")) != -1) {
        switch (opt) {
            case 'h':
                display_help_menu();
                exit(EXIT_SUCCESS);
                break;
            case 'j':
                job_thread_count = atoi(optarg);
                j_specified = 1;
                break;
            case 't':
                tick_len_seconds = atoi(optarg);
                t_specified = 1;
                break;
        }
    }

    for (; optind < argc; optind++) {
        printf("optind = %d\n", optind);
        printf("argv[optind] = %s\n", argv[optind]);
        printf("Entered access block\n");
        printf("extra_arg_idx = %d\n", extra_arg_idx);
        if (extra_arg_idx == 0) {
            server_port = atoi(argv[optind]);
        }
        else if (extra_arg_idx == 1) {
            auction_filename = argv[optind];
        }
        extra_arg_idx++;
    }

    printf("server_port = %d\n", server_port);
    printf("auction_filename = %s\n", auction_filename);
    printf("extra_arg_idx = %d\n", extra_arg_idx);
    if (extra_arg_idx != 2) {
        fprintf(stderr, "ERROR: Missing argument for PORT_NUMBER and/or AUCTION_FILENAME\n");
        display_help_menu();
        exit(EXIT_FAILURE);
    }

    if (!j_specified) {
        job_thread_count = 2;
    }
    if (!t_specified) {
        printf("Indicate seconds between time ticks: ");

        char c = '\0';
        char* word = malloc(sizeof(char));
        void* reallocResult = NULL;
        size_t wordSize = sizeof(char);
        int wordLen = wordSize / sizeof(char);

        while ((c = getchar()) != '\n' && (reallocResult = realloc(word, wordSize+sizeof(char))) != NULL) {
            printf("c = %c\n", c);
            wordSize++; 
            wordLen = wordSize;
            word = reallocResult;
            word[wordLen-2] = c;
            word[wordLen-1] = '\0';
        }

        tick_len_seconds = atoi(word);
        printf("tick_len_seconds = %d\n", tick_len_seconds);
        free(word);
    }

    init_auctions(auction_filename);

    run_server(server_port, job_thread_count);

    return 0;
}
