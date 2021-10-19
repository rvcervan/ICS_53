#ifndef SERVER_H
#define SERVER_H

#include <arpa/inet.h>
#include <getopt.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

#include <fcntl.h>
#include <semaphore.h>

#include "linkedList.h"
// #include "csapp.h"
#include "protocol.h"

#define BUFFER_SIZE 1024
#define SA struct sockaddr

/*
Purpose of users_mutex and auctions_mutex
the mutex is to protect readcnt which keeps track of the number of readers
we release the mutex right after incrementing readcnt and then we actually do the reading
once the reading is done we grab the mutex and decrement readcnt and release it again
*/

/*
reader/writer locking model for Users list
*/
List_t* Users;
int users_readcnt;
sem_t users_mutex;
sem_t users_write_lock; 

/*
reader/writer locking model for Auction list
*/
List_t* Auctions;
int auctions_readcnt;
sem_t auctions_mutex;
sem_t auctions_write_lock; 

/*
Socket for listening request
*/
int listen_fd;

typedef struct Msg {
    int client_fd;
    int msg_type;
    int msg_len;
    char* msg;
} Msg_t;

/*
custom sbuf_t for job queue
*/
typedef struct {
    Msg_t* buf; /* Buffer array */
    int n; /* Maximum number of slots */
    int front; /* buf[(front+1)%n] is first item */
    int rear; /* buf[rear%n] is last item */
    sem_t mutex; /* Protects accesses to buf */
    sem_t slots; /* Counts available slots */
    sem_t items; /* Counts available items */
} sbuf_t;

/*
Job Queue
*/
sbuf_t Job_Queue;

/*
mutex Auction ID
(starting at 1) indicating the next available ID for a newly created auction
*/
int aid;
sem_t aid_lock;


/*
if ctrl-c is pressed, all sockets are closed correctly
 */
void server_sigint_handler();

void run_server(int server_port, int job_thread_count);

/*
Initiate server and start listening on specified port
 */
int server_init(int server_port);

/*
Create threads to process tasks
 */
void run_client(char *server_addr_str, int server_port);

void display_help_menu();

/*
Prefill server with auctions
Each line corresponds to one auction item
Line format matches msg body used in ANCREATE protocol
Auctions are separated by "\n\n" and will always end in "\n\n"
 */
void init_auctions(char* auction_filename);

Auction_t* create_auction_node(char* item_name, int duration, int bin_price);
void* Green_Thread(void* vargp);
int isValidFormat(char* msg, int msg_len, int delimiter_count);
void Parse_ANBID(char* msg, int msg_len, int* auction_id, int* bid);
void Parse_ANCREATE(char* msg, int msg_len, char* item_name, int* duration, int* bin_price);
void* Red_Thread(void* vargp);
void sbuf_init(sbuf_t *sp, int n);
void sbuf_deinit(sbuf_t *sp);
void sbuf_insert(sbuf_t *sp, Msg_t* item);
Msg_t* sbuf_remove(sbuf_t *sp);
void Parse_Login(char* username, char* password, char* login_req, int req_len);
User_t* CreateAccount(char* username, char* password, int fd);
int AuctionIDComparator(void* lhs, void* rhs);
int UserComparator(void* lhs, void* rhs);
User_t* FindUser(int client_fd);
void WriteReplyMsg(char* reply_msg, int* reply_msg_len, char* content);
int GetWatchersCount(List_t* watcherlist_head);
char* int_to_str(int x);
void display_help_menu();
Auction_t* FindAuction(int aid);


Auction_t* create_auction_node(char* item_name, int duration, int bin_price) {
    Auction_t* new_node = malloc(sizeof(Auction_t));
    new_node->aid = aid;
    new_node->creator_username = "";
    new_node->ticks_remain = duration;
    new_node->watcherlist_head = NULL;
    new_node->item_name = item_name;
    new_node->bin_price = bin_price;
    new_node->highest_bid = 0;

    List_t* users_watching = malloc(sizeof(List_t));
    users_watching->head = NULL;
    users_watching->length = 0;
    users_watching->comparator = NULL;
    new_node->watcherlist_head = users_watching;

    return new_node;
}

void* Green_Thread(void* vargp) {
    // TODO: Consider using while(1) for sending jobs over time
    
        GTA_t* green_thread_args = vargp;
    while(1){
        int client_fd = green_thread_args->client_fd;
        int listen_fd = green_thread_args->listen_fd;
   
        printf("client_fd = %d and listen_fd = %d in green thread\n", client_fd, listen_fd);

        petr_header* msg_header = malloc(sizeof(msg_header));

        //printf("The rd_msgheader error is here in green thread?\n");

        rd_msgheader(client_fd, msg_header);
        char* msg = malloc(sizeof(char)*(msg_header->msg_len));
        read(listen_fd, msg, sizeof(char)*(msg_header->msg_len));
        int msg_type = msg_header->msg_type;
        int msg_len = msg_header->msg_len;

        //free(green_thread_args); green_thread_args was never malloced. commented out

        Msg_t* job_msg = malloc(sizeof(Msg_t));
        job_msg->client_fd = client_fd;
        job_msg->msg_type = msg_type;
        job_msg->msg_len = msg_len;
        job_msg->msg = msg;
    
        // 0 (check msg len) : 10, 11, 23, 24/ , 25/, 32, 33, 34, 35
        // 1 : 26
        // 2 : 20 

        //use strtok to check for null if isValidFormat is buggy
        if (
            msg_type == 0x23 ||
            (msg_type == 0x24 && msg_len > 0) ||
            (msg_type == 0x25 && msg_len > 0) ||
            msg_type == 0x32 ||
            msg_type == 0x33 ||
            msg_type == 0x34 ||
            msg_type == 0x35
        ) {
            if (isValidFormat(msg, msg_len, 0)) {
                printf("job with 0 delimits\n");
                sbuf_insert(&Job_Queue, job_msg);
            }
        }
        else if (msg_type == 0x11){
            if (isValidFormat(msg, msg_len, 0)){
                printf("log out\n");
                sbuf_insert(&Job_Queue, job_msg);
                pthread_exit(NULL);
            }
        }
        else if (msg_type == 0x26) {
            if (isValidFormat(msg, msg_len, 1)) {
                sbuf_insert(&Job_Queue, job_msg);
            }
        }
        else if (msg_type == 0x20) {
            if (isValidFormat(msg, msg_len, 2)) {
                sbuf_insert(&Job_Queue, job_msg);
            }
        }
        free(msg_header);
        free(msg);
    }
    return NULL;
}

int isValidFormat(char* msg, int msg_len, int delimiter_count) {
    int count = 0;
    int i = 0;
    for (i=0; i<msg_len; i++) {
        if (msg[i] == '\r') {
            count++;
            if (count > delimiter_count) {
                return 0;
            }
        }
    }
    if (count == delimiter_count) {
        return 1;
    }
    return 0;
}

void Parse_ANBID(char* msg, int msg_len, int* auction_id, int* bid) {
    int i = 0;
    char* bid_ptr = NULL;
    while (i<msg_len) {
        if (msg[i] == '\r') {
            msg[i] = '\0';
            i += 2;
            bid_ptr = &(msg[i]);
        }
        i++;
    }
    *auction_id = atoi(msg);
    *bid = atoi(bid_ptr);
}

void Parse_ANCREATE(char* msg, int msg_len, char* item_name, int* duration, int* bin_price) {
    char* duration_ptr = NULL;
    char* bin_price_ptr = NULL;
    int i = 0;
    int count = 0;
    while (i<msg_len) {
        if (msg[i] == '\r') {
            msg[i] = '\0';
            i += 2;
            if (count == 0) {
                item_name = &(msg[i]);
            }
            else if (count == 1) {
                duration_ptr = &(msg[i]);
            }
            else if (count == 2) {
                bin_price_ptr = &(msg[i]);
            }
            count++;
        }
        i++;
    }
    *duration = atoi(duration_ptr);
    *bin_price = atoi(bin_price_ptr);
}


// TODO: Finish implementing green thread, comeback when understand type of item in Job Queue
void* Red_Thread(void* vargp) {
    int retcode;
    //detached thread
    
    if ((retcode = pthread_detach(pthread_self())) != 0) {
        fprintf(stderr, "pthread_detach Red_Thread error: %s\n", strerror(retcode));
        exit(0);
    }

    printf("Doing Job\n");
    Msg_t* job_msg = NULL;
    char* msg;
    int msg_type;
    int msg_len;
    int client_fd;
    while (1) { 
        printf("inside jobs while loop\n");
        job_msg = sbuf_remove(&Job_Queue);   // Wait for a job in Job Queue
        
        //if another user sends a job would this get overwritten before current user gets his job response?
        // TODO: might need to do a mutex if that is true, lock->process job->unlock.
        msg = job_msg->msg;
        msg_type = job_msg->msg_type;
        msg_len = job_msg->msg_len;
        client_fd = job_msg->client_fd;

        //printf("msg=%s, msg_type=%d, msg_len=%d, client_fd=%d\n", msg, msg_type, msg_len, client_fd);
        
        // *** LOGOUT ***
        if (msg_type == 0x11) {
            printf("inside log out if statement\n");
            User_t* cur_val = FindUser(client_fd);

            char* username = cur_val->username;
            // go to all auctions the client is watching, remove client from all auctions' watcher list, lock when read/write
            sem_wait(&auctions_write_lock); // lock for writing Auctions
            node_t* cur_auc_node = Auctions->head;
            printf("after sem_wait\n");
            while (cur_auc_node != NULL) {
                List_t* watcherlist_head = ((Auction_t*)(cur_auc_node->value))->watcherlist_head;
                removeByName(watcherlist_head, username);
                cur_auc_node = cur_auc_node->next;\
                printf("kdkdkd\n");
            }
            sem_post(&auctions_write_lock); // done writing Auctions
            cur_val->isOnline = 0;

            // send rep to client_fd
            petr_header logout_rep_header;
            logout_rep_header.msg_len = 0;
            logout_rep_header.msg_type = OK;
            wr_msg(cur_val->fd, &logout_rep_header, NULL);
            printf("end of logout if statement\n");
        } 
        
        // *** ANLIST ***
        else if (msg_type == 0x23) {
            // order auction by <auction_id> alphabetically ascending
            // newline at end of each auction
            // \n\0 at end of message

            User_t* cur_val = FindUser(client_fd);//get the user account data
    
            char* reply_msg = NULL;
            
            // Increase num of Auctions readers
            sem_wait(&auctions_mutex);
            auctions_readcnt++;
            if (auctions_readcnt == 1) {
                sem_wait(&auctions_write_lock);
            }
            sem_post(&auctions_mutex);

            node_t* cur_auction_node = Auctions->head;
            //malloc string with starting space
            reply_msg = calloc(100, sizeof(char));
            int cond = 50;
            if (cur_auction_node != NULL) {
                // have auctions running
                // Iterate auctions, 
                Auction_t* cur_auction = NULL;
                char number[100];

                while (cur_auction_node != NULL) {
                    // write all info into reply_msg
                    cur_auction = cur_auction_node->value;
                    //format is "<auction_id>;<item_name>;<bin_price>;num_watchers>;<highest_bid>;
                    //<remaining_cycles>\n...

                    sprintf(number, "%d", cur_auction->aid);
                    strcat(reply_msg, number); //aid
                    strcat(reply_msg, ";");
                    memset(number, 0, 20);

                    strcat(reply_msg, cur_auction->item_name); //item_name
                    strcat(reply_msg, ";");
                    
                    
                    sprintf(number, "%d", cur_auction->bin_price);
                    strcat(reply_msg, number); //bin_price
                    strcat(reply_msg, ";");
                    memset(number, 0, 20);

                    //sprintf(number, "%d", ((List_t*)(cur_auction->watcherlist_head))->length); error?
                    sprintf(number, "%d", 12);
                    strcat(reply_msg, number); //num_watchers
                    strcat(reply_msg, ";");
                    memset(number, 0, 20);

                    sprintf(number, "%d", cur_auction->highest_bid);
                    strcat(reply_msg, number); //highest_bid
                    strcat(reply_msg, ";");
                    memset(number, 0, 20);

                    sprintf(number, "%d", cur_auction->ticks_remain);
                    strcat(reply_msg, number); //ticks_remain
                    strcat(reply_msg, ";");
                    memset(number, 0, 20);

                    strcat(reply_msg, "\n");
                    
                    cur_auction_node = cur_auction_node->next;
                    //realloc string to obtain more space
                    if(strlen(reply_msg) > cond){
                        reply_msg = realloc(reply_msg, (cond + 100)); //increase space by 50
                        cond = cond + 50;
                    }

                }
               
                //Add \0 at the end of message
                strcat(reply_msg, "\0");
            }
            else{//auctions is null
                printf("Auctions list is Null/empty");
            }
            //else: cur_auction_node is NULL, no auctions currently running

            // Decrease num of Auctions readers
            sem_wait(&auctions_mutex);
            auctions_readcnt--;
            if (auctions_readcnt == 0) {
                sem_post(&auctions_write_lock);
            }
            sem_post(&auctions_mutex);  // Done reading Auctions
            
            // send rep to client_fd
            petr_header anlist_rep_header;
            anlist_rep_header.msg_len = strlen(reply_msg); //Handles empty auction list by default
            anlist_rep_header.msg_type = ANLIST;
            wr_msg(cur_val->fd, &anlist_rep_header, reply_msg);
            //free reply_msg
            free(reply_msg);
        }
        else if (msg_type == 0x24) { //ANWATCH 
            //msg = <auction_id>
            //adds user (account?) to watcherlist_head in the Auction_t struct

            //find auction by id and get auction
            //create a pointer to point to user account that requested watch

            //lock auctionlist
            //insert user pointer into watcherlist_head
            //unlock auctionlist

            //return <item_name>\r\n<bin_price>

            User_t* user = FindUser(client_fd);
            // TODO: send ANLEAVE when the xterm window of client is terminated/closed  

            node_t* cur_auction_node = Auctions->head;

            Auction_t* cur_auction = NULL;

            if(cur_auction_node != NULL){

                while(cur_auction_node != NULL){
                    cur_auction = cur_auction_node->value;
                        if(cur_auction->aid == atoi(msg)){//found
                            break;
                        }
                    cur_auction_node = cur_auction_node->next;
                }

            }

            petr_header anwatch_rep_header;
            char* reply_msg = calloc(100, sizeof(char));

            if(cur_auction == NULL){//auction with ID does not exist
                //respond with EANNOTFOUND
                anwatch_rep_header.msg_len = 0;
                anwatch_rep_header.msg_type = EANNOTFOUND;
                wr_msg(user->fd, &anwatch_rep_header, reply_msg);

                free(job_msg);
                break;
            }
            //if auction has reached defined max of concurrent watchers, (min 5) then the server responds
            //with EANFULL. EANFULL will never be sent if the server supports infinite watchers.

            //lock
            sem_wait(&auctions_write_lock);
            insertFront(cur_auction->watcherlist_head, user);
            sem_post(&auctions_write_lock);
            //unlock
            char number[50];
            strcat(reply_msg, cur_auction->item_name);
            strcat(reply_msg, "\r\n");

            sprintf(number, "%d", cur_auction->bin_price);
            strcat(reply_msg, number);
            strcat(reply_msg, "\0");

            anwatch_rep_header.msg_len = strlen(reply_msg);
            anwatch_rep_header.msg_type = ANWATCH;
            wr_msg(user->fd, &anwatch_rep_header, reply_msg);
            
            free(reply_msg);
        }

        // *** ANLEAVE ***
        else if (msg_type == 0x25) {
            // msg : <auction_id>
            Auction_t* cur_auction = FindAuction(atoi(msg));
            User_t* cur_user = FindUser(client_fd);
            petr_header rep_header;

            //malloc string with len of msg + 1 for null terminator
            char* reply_msg = calloc(100, (strlen("Auction does not exists on server.")+1) * sizeof(char));

            // Error case: Auction does not exist, send EANNOTFOUND
            if (cur_auction == NULL) {
                strcat(reply_msg, "Auction does not exists on server.");

                // send rep to client_fd
                rep_header.msg_len = strlen(reply_msg);
                rep_header.msg_type = EANNOTFOUND;
                wr_msg(cur_user->fd, &rep_header, reply_msg);
            }
            else {
                // OK cases: Auction exists, no matter user is watching or not
                // Remove user from the watch list of the item
                removeByName(cur_auction->watcherlist_head, cur_user->username);
                // send OK
                rep_header.msg_len = 0;
                rep_header.msg_type = OK;
                wr_msg(cur_user->fd, &rep_header, NULL);
            }
 
            //free reply_msg
            free(reply_msg);
        }

        // *** USRLIST ***
        else if (msg_type == 0x32) {
            User_t* cur_user = FindUser(client_fd);
            petr_header rep_header;
            rep_header.msg_len = 0;
            char* msg_buf = NULL;

            // get active usernames other than the requesting user
            // build msg body

            // respond USRLIST to client
            rep_header.msg_type = USRLIST;
            wr_msg(cur_user->fd, &rep_header, msg_buf);
        }

        else if (msg_type == 0x33) {
            // parse msg
        }
        else if (msg_type == 0x34) {
            // parse msg
        }
        else if (msg_type == 0x35) {
            // parse msg
        }
        else if (msg_type == 0x26) {
            // parse msg
            int auction_id = 0;
            int bid = 0;
            Parse_ANBID(msg, msg_len, &auction_id, &bid);
        }
        else if (msg_type == 0x20) {
            // parse msg
            char* item_name = NULL;
            int duration = 0;
            int bin_price = 0;
            Parse_ANCREATE(msg, msg_len, item_name, &duration, &bin_price);
        }
    
    }
    free(job_msg);
    //free job msghere?
    return NULL;
}






/* Create an empty, bounded, shared FIFO buffer with n slots */
void sbuf_init(sbuf_t *sp, int n)
{
    sp->buf = calloc(n, sizeof(Msg_t));
    sp->n = n;                      /* Buffer holds max of n items */
    sp->front = sp->rear = 0;    /* Empty buffer iff front == rear */
    sem_init(&sp->mutex, 0, 1);     /* Binary semaphore for locking */
    sem_init(&sp->slots, 0, n);     /* Initially, buf has n empty slots */
    sem_init(&sp->items, 0, 0);     /* Initially, buf has 0 items */
}

/* Clean up buffer sp */
void sbuf_deinit(sbuf_t *sp)
{
    free(sp->buf);
}

/* Insert item onto the rear of shared buffer sp */
void sbuf_insert(sbuf_t *sp, Msg_t* item)
{
    sem_wait(&sp->slots);                              /* Wait for available slot */
    sem_wait(&sp->mutex);                              /* Lock the buffer */
    sp->buf[(++sp->rear)%(sp->n)] = *item;       /* Insert the item */
    sem_post(&sp->mutex);                              /* Unlock the buffer */
    sem_post(&sp->items);                              /* Announce available item */
}

/* Remove and return the first item from buffer sp */
Msg_t* sbuf_remove(sbuf_t *sp)
{
    Msg_t* item = NULL;
    sem_wait(&sp->items);                              /* Wait for available item */
    sem_wait(&sp->mutex);                              /* Lock the buffer */
    item = &(sp->buf[(++sp->front)%(sp->n)]);      /* Remove the item */
    sem_post(&sp->mutex);                              /* Unlock the buffer */
    sem_post(&sp->slots);                              /* Announce available slot */

    return item;
}

void Parse_Login(char* username, char* password, char* login_req, int req_len) {
    /*
    int i=0;
    int username_len = 0;
    int password_len = 0;
    char* password_start_address = NULL;
    int found_username = 0;
    while (i<req_len) {
        if (login_req[i] == '\r') {
            login_req[i] = '\0';
            found_username = 1;
            i++;
            password_start_address = &(login_req[i]);
        }
        if (!found_username) {
            username_len++;
        }
        else {
            password_len++;
        }
        i++;
    }
    username = malloc(username_len);
    strcpy(username, login_req);
    password = malloc(password_len);
    strcpy(password, login_req);
    */


    printf("Username=%s, Password=%s\n", username, password);
}

User_t* CreateAccount(char* username, char* password, int fd) {
    User_t* new_account = malloc(sizeof(User_t));
    new_account->auctionlist_head = NULL;
    new_account->fd = fd;
    new_account->username = username;
    new_account->password = password;
    new_account->isOnline = 1;
    return new_account;
}

int AuctionIDComparator(void* lhs, void* rhs) {
    int lhs_aid = ((Auction_t*)lhs)->aid;
    int rhs_aid = ((Auction_t*)rhs)->aid;

    if (lhs_aid < rhs_aid)
    {
        return -1;
    }
    else if (lhs_aid == rhs_aid)
    {
        return 0;
    }
    return 1;
}

User_t* FindUser(int client_fd) {
    // Lock for reading Users
    sem_wait(&users_mutex);
    users_readcnt++;
    if (users_readcnt == 1) {
        sem_wait(&users_write_lock);
    }
    sem_post(&users_mutex);

    node_t* cur_node = Users->head;
    User_t* cur_val;
    while (cur_node != NULL) {  // Find the username that requested
        printf("Finding User\n");
        cur_val = (User_t*)(cur_node->value);

        if (cur_val->isOnline == 1 && client_fd == cur_val->fd) {
            sem_wait(&users_mutex);
            users_readcnt--;
            if (users_readcnt == 0) {
                sem_post(&users_write_lock);
            }
            sem_post(&users_mutex);
            // Done reading Users
            return cur_val;
        }

        cur_node = cur_node->next;
    }

    sem_wait(&users_mutex);
    users_readcnt--;
    if (users_readcnt == 0) {
        sem_post(&users_write_lock);
    }
    sem_post(&users_mutex); // Done reading Users
    
    return NULL;
}

Auction_t* FindAuction(int aid) {
    // Lock for reading Auctions
    sem_wait(&auctions_mutex);
    auctions_readcnt++;
    if (auctions_readcnt == 1) {
        sem_wait(&auctions_write_lock);
    }
    sem_post(&auctions_mutex);

    node_t* cur_node = Auctions->head;
    Auction_t* cur_val;
    while (cur_node != NULL) { // Find the auction with the aid requested
        printf("Finding Auction\n");
        cur_val = (Auction_t*)(cur_node->value);

        if (aid == cur_val->aid) {

            // Decrease num of Auctions readers
            sem_wait(&auctions_mutex);
            auctions_readcnt--;
            if (auctions_readcnt == 0) {
                sem_post(&auctions_write_lock);
            }
            sem_post(&auctions_mutex);  // Done reading Auctions

            return cur_val;
        }
        cur_node = cur_node->next;
    }

    // Decrease num of Auctions readers
    sem_wait(&auctions_mutex);
    auctions_readcnt--;
    if (auctions_readcnt == 0) {
        sem_post(&auctions_write_lock);
    }
    sem_post(&auctions_mutex);  // Done reading Auctions

    return NULL;
}


/*
void WriteReplyMsg(char* reply_msg, int* reply_msg_len, char* content) {
    *reply_msg_len += strlen(content);
    reply_msg = realloc(reply_msg, *reply_msg_len);
    strcat(reply_msg, content);
}
*/
int GetWatchersCount(List_t* watcherlist_head) {
    node_t* cur_watcher_node = watcherlist_head->head;
    int count = 0;
    while (cur_watcher_node != NULL) {
        count++;
        cur_watcher_node = cur_watcher_node->next;
    }
    return count;
}

/*
char* int_to_str(int x) {
    //char* result_str = malloc(sizeof(char) * sizeof(int) * 4 + 1);
    char result_str[10];
    sprintf(result_str, "%d", x);
    
    return result_str;
}
*/
void display_help_menu() {
    printf("./bin/zbid+server [-h] [-j N] [-t M] PORT_NUMBER AUCTION_FILENAME\n\n");
    printf("-h                 Displays this help menu, and returns EXIT_SUCCESS.\n");
    printf("-j N               Number of job threads. If option not specified, default to 2.\n");
    printf("-t M               M seconds between time ticks. If option not specified, default is to wait on input from stdin to indicate a tick.\n");
    printf("PORT_NUMBER        Port number to listen on.\n");
    printf("AUCTION_FILENAME   File to read auction item information from at the start of the server.\n");
}

                


#endif  // SERVER_H
