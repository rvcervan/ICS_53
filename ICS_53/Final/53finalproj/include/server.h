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
#include <math.h>

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
    int client_fd;  // if msg_typ is ANCLOSED, client_fd is the target's aid
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
int aid = 1;
sem_t aid_lock;


/*
if ctrl-c is pressed, all sockets are closed correctly
 */
void server_sigint_handler();

void run_server(int server_port, int job_thread_count, int is_realtime_mode, int tick_len_seconds);

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

Auction_t* create_auction_node(char* creator_name, char* item_name, int duration, int bin_price);
void* Green_Thread(void* vargp);
int isValidFormat(char* msg, int msg_len, int delimiter_count);
void Parse_ANBID(char* msg, int msg_len, int* auction_id, int* bid);
void Parse_ANCREATE(char* msg, int msg_len, char* item_name, int* duration, int* bin_price);
void* Red_Thread(void* vargp);
void* Yellow_Thread(void* vargp);
void sbuf_init(sbuf_t *sp, int n);
void sbuf_deinit(sbuf_t *sp);
void sbuf_insert(sbuf_t *sp, Msg_t* item);
Msg_t* sbuf_remove(sbuf_t *sp);
void Parse_Login(char* username, char* password, char* login_req, int req_len);
User_t* CreateAccount(char* username, char* password, int fd);
int AuctionIDComparator(void* lhs, void* rhs);
int UserComparator(void* lhs, void* rhs);
User_t* FindUserFD(int client_fd);
User_t* FindUserName(char* username);
User_t* FindWatcher(Auction_t* auction, int client_fd);
void WriteReplyMsg(char* reply_msg, int* reply_msg_len, char* content);
int GetWatchersCount(List_t* watchers_list);
char* int_to_str(int x);
void display_help_menu();
Auction_t* FindAuction(int aid);
void delete_file_auction(node_t* auction);
// int intlen(int n);


Auction_t* create_auction_node(char* creator_name, char* item_name, int duration, int bin_price) {
    Auction_t* new_node = malloc(sizeof(Auction_t));
    new_node->aid = aid;
    new_node->creator_username = creator_name;
    new_node->ticks_remain = duration;
    new_node->watchers_list = NULL;
    new_node->item_name = item_name;
    new_node->bin_price = bin_price;
    new_node->highest_bid = 0;

    List_t* users_watching = malloc(sizeof(List_t));
    users_watching->head = NULL;
    users_watching->length = 0;
    users_watching->comparator = NULL;
    new_node->watchers_list = users_watching;

    return new_node;
}

void* Green_Thread(void* vargp) {
    
    GTA_t* green_thread_args = vargp;
    while(1){
        int client_fd = green_thread_args->client_fd;
        int listen_fd = green_thread_args->listen_fd;
   
        //printf("client_fd = %d and listen_fd = %d in green thread\n", client_fd, listen_fd);

        petr_header* msg_header = malloc(sizeof(msg_header));

        //printf("The rd_msgheader error is here in green thread?\n");
        int F;
        F = rd_msgheader(client_fd, msg_header);
        printf("rd_msgheader returns %d\n", F);
        char* msg = malloc(sizeof(char)*(msg_header->msg_len));
        read(client_fd, msg, sizeof(char)*(msg_header->msg_len));
        printf("After the read call\n");
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
               
            printf("job with 0 delimits\n");
            sbuf_insert(&Job_Queue, job_msg);
            
        }
        else if (msg_type == 0x11){
            
            printf("log out\n");
            sbuf_insert(&Job_Queue, job_msg);
            pthread_exit(NULL);
          
        }
        else if (msg_type == 0x26) {
            
            sbuf_insert(&Job_Queue, job_msg);
            
        }
        else if (msg_type == 0x20) {
            printf("ANCREATE GREEN\n");
            
            sbuf_insert(&Job_Queue, job_msg);

        }
        free(msg_header);
        //free(msg);
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
        // We don't need mutex because each job_msg is local to only its own Red Thread.
        // Other Red Threads cannot change the value of job_msg in a Red Thread without having access to it
        // through a pointer

        msg = job_msg->msg;
        msg_type = job_msg->msg_type;
        msg_len = job_msg->msg_len;
        client_fd = job_msg->client_fd;

        //printf("msg=%s, msg_type=%d, msg_len=%d, client_fd=%d\n", msg, msg_type, msg_len, client_fd);
        
        // *** LOGOUT ***
        if (msg_type == 0x11) {
            printf("inside log out if statement\n");
            User_t* requesting_user = FindUserFD(client_fd);

            char* username = requesting_user->username;
            // go to all auctions the client is watching, remove client from all auctions' watcher list, lock when read/write
            sem_wait(&auctions_write_lock);
            node_t* cur_auc_node = Auctions->head;

            while (cur_auc_node != NULL) {
                List_t* watchers_list = ((Auction_t*)(cur_auc_node->value))->watchers_list;
                removeByName(watchers_list, username);
                cur_auc_node = cur_auc_node->next;
            }
            sem_post(&auctions_write_lock);
            requesting_user->isOnline = 0;

            // send rep to client_fd
            petr_header logout_rep_header;
            logout_rep_header.msg_len = 0;
            logout_rep_header.msg_type = OK;
            wr_msg(requesting_user->fd, &logout_rep_header, NULL);
            printf("end of logout if statement\n");
            close(requesting_user->fd);
        } 
        // *** ANCREATE ***
        else if (msg_type == 0x20) {
            // parse msg
            //printf("%s", msg);
            printf("begin ANCREATE job\n");
            char* msg_rest = msg;

            char* item_name = NULL;

            char* dur_str = NULL;
            int duration = 0;

            char* bin_str = NULL;
            int bin_price = 0;

            User_t* user = FindUserFD(client_fd);
            
            
            petr_header ancreate_rep_header;
            //msg = <item_name>\r\n<duration>\r\n<bin_price>
            item_name = strtok_r(msg_rest, "\r\n", &msg_rest);
            if(item_name == NULL){
                printf("item_name is NULL\n");
                ancreate_rep_header.msg_len = 0;
                ancreate_rep_header.msg_type = ESERV;
                wr_msg(user->fd, &ancreate_rep_header, NULL);
                free(job_msg->msg);
                free(job_msg);
                continue;
                
            }

            dur_str = strtok_r(msg_rest, "\r\n", &msg_rest);
            if(dur_str == NULL){
                printf("dur_str is NULL\n");
                ancreate_rep_header.msg_len = 0;
                ancreate_rep_header.msg_type = ESERV;
                wr_msg(user->fd, &ancreate_rep_header, NULL);
                free(job_msg->msg);
                free(job_msg);
                continue;
                
            }
            duration = atoi(dur_str);
            
            bin_str = strtok_r(msg_rest, "\r\n", &msg_rest);
            if(bin_str == NULL){
                printf("bin_str is NULL\n");
                ancreate_rep_header.msg_len = 0;
                ancreate_rep_header.msg_type = ESERV;
                wr_msg(user->fd, &ancreate_rep_header, NULL);
                free(job_msg->msg);
                free(job_msg);
                continue;
                
            }
            bin_price = atoi(bin_str);

             
            if(duration < 1 || bin_price < 0 || (strcmp(item_name, "")==0)){
                printf("EINVALIDARG\n");
                ancreate_rep_header.msg_len = 0;
                ancreate_rep_header.msg_type = EINVALIDARG;
                wr_msg(user->fd, &ancreate_rep_header, NULL);
                continue;
            }
            printf("Item_name is %s\n", item_name);

            Auction_t* new_auction = create_auction_node(user->username, item_name, 
                                                                    duration, bin_price);

             

            

            char* reply_msg = calloc(10, sizeof(char));

            sprintf(reply_msg, "%d", aid);
            strcat(reply_msg, "\0");
            sem_post(&aid_lock);
            aid++;
            sem_wait(&aid_lock);

            sem_post(&auctions_write_lock);
            insertRear(Auctions, new_auction);
            sem_wait(&auctions_write_lock);
            
            ancreate_rep_header.msg_len = strlen(reply_msg)+1;
            ancreate_rep_header.msg_type = ANCREATE;
            
            wr_msg(user->fd, &ancreate_rep_header, reply_msg );
            printf("ANCREATE\n");
            free(reply_msg);
        }

        // *** ANCLOSED ***
        //TODO: handle auctions with no creator name from input init file.
        else if (msg_type == 0x22) {
            // client_fd = aid of the auction being closed
            int aid = client_fd;

            sem_wait(&auctions_write_lock);
            node_t* removed_auction_node = removeByAid(Auctions, aid);
            sem_post(&auctions_write_lock);

            Auction_t* removed_auction = removed_auction_node->value;

            char* creator_name = removed_auction->creator_username;

            User_t* creator = FindUserName(creator_name);
            

            // Reply to all watchers of removed_auction
            char* reply_msg = NULL;
            int msg_buf_size = 1;

            int auction_id = removed_auction->aid;
            char* win_name = removed_auction->highest_bidder;
            int win_price = removed_auction->highest_bid;
            
            if (win_name == NULL) {  // No winner
                // format: <auction_id>\r\n\r\n
                msg_buf_size += 50; // auction_id
                msg_buf_size += 4;  // \r\n\r\n

                reply_msg = realloc(reply_msg, msg_buf_size);
                sprintf(reply_msg, "%d\r\n\r\n", auction_id);
            }
            else {  // Has winner
                // format: <auction_id>\r\n<win_name>\r\n<win_price>\r\n
                msg_buf_size += 6; // 3 pairs of \r\n
                msg_buf_size += 50; // auction_id
                msg_buf_size += strlen(win_name);   // win_name
                msg_buf_size += 50;  // win_price

                reply_msg = realloc(reply_msg, msg_buf_size);
                sprintf(reply_msg, "%d\r\n%s\r\n%d\r\n", auction_id, win_name, win_price);
            }

            // send rep to client_fd
            petr_header rep_header;
            rep_header.msg_len = strlen(reply_msg); //Handles empty auction list by default
            rep_header.msg_type = ANCLOSED;

            node_t* cur_node = (removed_auction->watchers_list)->head;
            User_t* cur_watcher = NULL;
            while (cur_node != NULL) {
                cur_watcher = cur_node->value;
                wr_msg(cur_watcher->fd, &rep_header, reply_msg);

                cur_node = cur_node->next;
            }

            
            if(creator != NULL){
                List_t* creator_sold_list = creator->sold_auctions_list;
                insertRear(creator_sold_list, removed_auction_node);
            }
            else{
                //delete auction node;
                delete_file_auction(removed_auction_node);
            }
            
            //free reply_msg
            free(reply_msg);
        }
        
        // *** ANLIST ***
        else if (msg_type == 0x23) {
            // order auction by <auction_id> alphabetically ascending
            // newline at end of each auction
            // \n\0 at end of message

            User_t* requesting_user = FindUserFD(client_fd);//get the user account data
            
            // Increase num of Auctions readers
            sem_wait(&auctions_mutex);
            auctions_readcnt++;
            if (auctions_readcnt == 1) {
                sem_wait(&auctions_write_lock);
            }
            sem_post(&auctions_mutex);

            //malloc string with starting space
            char* reply_msg = calloc(2,sizeof(char));
            int msg_buf_size = 1;   // starting with size for null terminator

            int auction_id;
            char* item_name = NULL;
            int bin_price;
            int num_watchers;
            int highest_bid;
            int remaining_cycles;

            

            node_t* cur_auction_node = Auctions->head;  
            if (cur_auction_node != NULL) {
                // have auctions running
                // Iterate auctions, 
                Auction_t* cur_auction = NULL;

                while (cur_auction_node != NULL) {
                    // write all info into reply_msg

                    cur_auction = cur_auction_node->value;
                    //format is "<auction_id>;<item_name>;<bin_price>;num_watchers>;<highest_bid>;<remaining_cycles>\n...

                    msg_buf_size += 6;  // 5 semicolon, 1 newline 

                    auction_id = cur_auction->aid;
                    // msg_buf_size += intlen(auction_id);
                    msg_buf_size += 250;

                    item_name = cur_auction->item_name;
                    msg_buf_size += strlen(item_name);

                    bin_price = cur_auction->bin_price;
                    // msg_buf_size += intlen(bin_price);

                    num_watchers = (cur_auction->watchers_list)->length;
                    // msg_buf_size += intlen(num_watchers);

                    highest_bid = cur_auction->highest_bid;
                    // msg_buf_size += intlen(highest_bid);

                    remaining_cycles = cur_auction->ticks_remain;
                    // msg_buf_size += intlen(remaining_cycles);
                    
                    cur_auction_node = cur_auction_node->next;

                    //realloc string to obtain more space
                    reply_msg = realloc(reply_msg, msg_buf_size);   // Can increment msg_buf_size to make sure no buffer overflow
                   
                    printf("item_name is %s\n", item_name);

                    char* temp = calloc(msg_buf_size, sizeof(char));
                    sprintf(temp, "%d;%s;%d;%d;%d;%d\n",
                            auction_id, 
                            item_name, 
                            bin_price, 
                            num_watchers, 
                            highest_bid, 
                            remaining_cycles);

                    strcat(reply_msg, temp);

                    free(temp);

                     
                }
                
            }
            else{//auctions is null
                printf("Auctions list is Null/empty");
            }
            //else: cur_auction_node is NULL, no auctions currently running
            strcat(reply_msg, "\0");
            // Decrease num of Auctions readers
            sem_wait(&auctions_mutex);
            auctions_readcnt--;
            if (auctions_readcnt == 0) {
                sem_post(&auctions_write_lock);
            }
            sem_post(&auctions_mutex);  // Done reading Auctions
            int msg_len;
            if(strlen(reply_msg) == 0){//empty string that doesn't count \0
                msg_len = 0;
            }
            else{
                msg_len = strlen(reply_msg)+1;
            }
            // send rep to client_fd
            petr_header rep_header;
            rep_header.msg_len = msg_len; //Handles empty auction list by default
            rep_header.msg_type = ANLIST;
            wr_msg(requesting_user->fd, &rep_header, reply_msg);
            //free reply_msg
            free(reply_msg);
        }

        // *** ANWATCH ***
        else if (msg_type == 0x24) { 
            //msg = <auction_id>
            // TODO: send ANLEAVE when the xterm window of client is terminated/closed 
            User_t* requesting_user = FindUserFD(client_fd);//find user

            petr_header rep_header;
            char* reply_msg = NULL;
            int msg_buf_size = 1;   // starting with size for null terminator

            node_t* cur_auction_node = Auctions->head;
            Auction_t* cur_auction = NULL;
            printf("Before if while\n");
            if(cur_auction_node != NULL){   // Auctions List is not empty
                while(cur_auction_node != NULL){
                    cur_auction = cur_auction_node->value;
                        if(cur_auction->aid == atoi(msg)){//found
                            break;
                        }
                    cur_auction_node = cur_auction_node->next;
                }
            }
            printf("After if while\n");
            if(cur_auction_node == NULL){        
                // Reached the end of Auctions Lists
                // auction with ID does not exist
                // respond with EANNOTFOUND
                rep_header.msg_len = 0;
                rep_header.msg_type = EANNOTFOUND;
                wr_msg(requesting_user->fd, &rep_header, reply_msg);

                continue;
            }


            //lock
            sem_wait(&auctions_write_lock);
            //insert user pointer into watchers_list
            insertRear(cur_auction->watchers_list, requesting_user);
            sem_post(&auctions_write_lock);
            //unlock
            printf("After auction lock\n");

            char* item_name = NULL;
            int bin_price;
            //format: <item_name>\r\n<bin_price>
            item_name = cur_auction->item_name;
            msg_buf_size += strlen(item_name);  // item_name

            msg_buf_size += 2;  // 2 for \r\n

            bin_price = cur_auction->bin_price; // bin_price
            // msg_buf_size += intlen(bin_price);
            msg_buf_size += 50;
     
            //realloc string to obtain more space
            reply_msg = calloc(msg_buf_size, sizeof(char));   // Can increment msg_buf_size to make sure no buffer overflow
            sprintf(reply_msg, "%s\r\n%d", item_name, bin_price);
            strcat(reply_msg, "\0");

            rep_header.msg_len = strlen(reply_msg)+1;
            rep_header.msg_type = ANWATCH;
            wr_msg(requesting_user->fd, &rep_header, reply_msg);
            
            free(reply_msg);
        }

        // *** ANLEAVE ***
        else if (msg_type == 0x25) {
            // msg : <auction_id>
            Auction_t* cur_auction = FindAuction(atoi(msg));
            User_t* requesting_user = FindUserFD(client_fd);
            petr_header rep_header;

            // Error case: Auction does not exist, send EANNOTFOUND
            if (cur_auction == NULL) {
                // malloc string with len of msg + 1 for null terminator
                // int msg_buf_size = strlen("Auction does not exists on server.") + 1;
                // char* reply_msg = realloc(reply_msg, msg_buf_size);
                
                // strcpy(reply_msg, "Auction does not exists on server.");

                // send rep to client_fd
                rep_header.msg_len = 0;
                rep_header.msg_type = EANNOTFOUND;
                wr_msg(requesting_user->fd, &rep_header, NULL);

                // free(reply_msg);
            }
            else {
                // OK cases: Auction exists, no matter user is watching or not
                // Remove user from the watch list of the item
                removeByName_NoFree(cur_auction->watchers_list, requesting_user->username);
                // send OK
                rep_header.msg_len = 0;
                rep_header.msg_type = OK;
                wr_msg(requesting_user->fd, &rep_header, NULL);
            }
        }

        // *** ANBID ***
        else if (msg_type == 0x26) {//ANBID
            // parse msg
            int auction_id = 0;
            int bid = 0;

            char* str_aid = strtok(msg, "\r\n");
            char* str_bid = strtok(NULL, "\r\n");

            auction_id = atoi(str_aid);
            bid = atoi(str_bid);

            //Find Auction
            User_t* requesting_user = FindUserFD(client_fd);

            node_t* cur_auction_node = Auctions->head;

            Auction_t* cur_auction = NULL;

            if(cur_auction_node != NULL){

                while(cur_auction_node != NULL){
                    cur_auction = cur_auction_node->value;
                        if(cur_auction->aid == auction_id){//found
                            break;
                        }
                    cur_auction_node = cur_auction_node->next;
                }

            }

            //If Auction not found, return EANNOTFOUND
            petr_header rep_header;
            if(cur_auction_node == NULL){
                rep_header.msg_len = 0;
                rep_header.msg_type = EANNOTFOUND;
                wr_msg(requesting_user->fd, &rep_header, NULL);
                continue;
            }

            //If auction exists but the user is not watching, server responds with EANDENIED
            User_t* watching = FindWatcher(cur_auction, client_fd);
            if(watching == NULL){
                rep_header.msg_len = 0;
                rep_header.msg_type = EANDENIED;
                wr_msg(requesting_user->fd, &rep_header, NULL);
                continue;
            }
            
            //if bidder name is same as auction creator name
            if(strcmp(requesting_user->username, cur_auction->creator_username)==0){
                rep_header.msg_len = 0;
                rep_header.msg_type = EANDENIED;
                wr_msg(requesting_user->fd, &rep_header, NULL);
                continue;
            }
            
            //if bid is lower than highest bid
            //might need to do a lock here since highest bid can change.
            if(bid <= cur_auction->highest_bid){
                rep_header.msg_len = 0;
                rep_header.msg_type = EBIDLOW;
                wr_msg(requesting_user->fd, &rep_header, NULL);
                continue;
            }

            // BIDDER IS AUTOMATICALLY WINNER Case:
            // if bid is greater or equal to bin_price, the auction is closed and ANCLOSED is also sent
            // to all users.
            char* reply_msg = NULL;
            int msg_buf_size = 1;
            if(bid >= cur_auction->bin_price){
                // format: <auction_id>\r\n<win_name>\r\n<win_price>\r\n
                msg_buf_size += 6; // 3 pairs of \r\n
                msg_buf_size += strlen(str_aid);
                char* win_name = requesting_user->username;
                msg_buf_size += strlen(win_name);
                msg_buf_size += strlen(str_bid);

                reply_msg = realloc(reply_msg, msg_buf_size);
                sprintf(reply_msg, "%s\r\n%s\r\n%s\r\n", str_aid, win_name, str_bid);
                
                rep_header.msg_len = strlen(reply_msg);
                rep_header.msg_type = ANCLOSED;
                
                node_t* watchers = cur_auction->watchers_list->head;
                while(watchers != NULL){
                    User_t* user_watching = watchers->value;
                    
                    wr_msg(user_watching->fd, &rep_header, reply_msg);  
                }

                free(reply_msg);
                continue;
            }

            // BIDDER IS NOT AUTOMATICALLY WINNER Case:
            //if bid, auction_id, and user are valid, the server responds with OK after updating
            //the auction data structure with the new highest bid value.
            sem_wait(&auctions_write_lock);
            cur_auction->highest_bid = bid;
            cur_auction->highest_bidder = requesting_user->username;
            sem_post(&auctions_write_lock);

            rep_header.msg_len = 0;
            rep_header.msg_type = OK;
            wr_msg(requesting_user->fd, &rep_header, NULL);

            //After the OK, server sends ANUPDATE message to all the users watching the auction.
            //Create ANUPDATE string
            // format: <auction_id>\r\n<item_name>\r\n<from_username>\r\n<bid>

            msg_buf_size += 6; // 3 pairs of \r\n
            msg_buf_size += strlen(str_aid);    // auction_id
            char* item_name = cur_auction->item_name;   // item_name 
            msg_buf_size += strlen(item_name);
            char* from_username = requesting_user->username; // from_username
            msg_buf_size += strlen(from_username);
            msg_buf_size += strlen(str_bid);    // bid

            reply_msg = realloc(reply_msg, msg_buf_size);
            sprintf(reply_msg, "%s\r\n%s\r\n%s\r\n%s", str_aid, item_name, from_username, str_bid);

            node_t* watchers = cur_auction->watchers_list->head;
            while(watchers != NULL){
                User_t* user_watching = watchers->value;
                if(strcmp(requesting_user->username, user_watching->username)!=0){
                    //sends ANUPDATE to user watching besides current bidder
                    rep_header.msg_len = strlen(reply_msg);
                    rep_header.msg_type = ANUPDATE;
                    wr_msg(user_watching->fd, &rep_header, reply_msg);
                }
            }

            free(reply_msg);
        }

        // *** USRLIST ***
        else if (msg_type == 0x32) {
            User_t* requesting_user = FindUserFD(client_fd);
            char* reply_msg = calloc(2, sizeof(char));;
            int msg_buf_size = 1;

            // get active usernames other than the requesting user
            // build msg body
            
            // Lock for reading Users
            sem_wait(&users_mutex);
            users_readcnt++;
            if (users_readcnt == 1) {
                sem_wait(&users_write_lock);
            }
            sem_post(&users_mutex);

            node_t* cur_node = Users->head;
            User_t* cur_user = NULL;
            while (cur_node != NULL) {
                // format is "<username>\n<username>\n...<username>\n\0"
                cur_user = cur_node->value;
                if (cur_user->username != requesting_user->username) {
                    
                    msg_buf_size += strlen(cur_user->username); // username
                    msg_buf_size += 1; // \n

                    reply_msg = realloc(reply_msg, msg_buf_size);
                    strcat(reply_msg, cur_user->username);
                    strcat(reply_msg, "\n");
                }

                cur_node = cur_node->next;
            }
            strcat(reply_msg, "\0");
            // Done reading Users
            sem_wait(&users_mutex);
            users_readcnt--;
            if (users_readcnt == 0) {
                sem_post(&users_write_lock);
            }
            sem_post(&users_mutex);

            // respond USRLIST to client
            petr_header rep_header;
            rep_header.msg_len = strlen(reply_msg)+1;
            rep_header.msg_type = USRLIST;
            wr_msg(requesting_user->fd, &rep_header, reply_msg);

            //free reply_msg
            free(reply_msg);
        }

        // *** USRWINS ***
        else if (msg_type == 0x33) {
            User_t* requesting_user = FindUserFD(client_fd);
            char* reply_msg = NULL;
            int msg_buf_size = 1;

            // Lock for reading Auctions
            sem_wait(&auctions_mutex);
            auctions_readcnt++;
            if (auctions_readcnt == 1) {
                sem_wait(&auctions_write_lock);
            }
            sem_post(&auctions_mutex);

            // Interate list of auctions won by requesting_user, get info into msg body
            node_t* cur_node = (requesting_user->won_auctions_list)->head;
            Auction_t* cur_auction;
            int auction_id;
            char* item_name = NULL;
            int winning_bid;
            while (cur_node != NULL) {
                // format is: <auction_id>;<item_name>;<winning_bid>\n ...
                cur_auction = cur_node->value;

                msg_buf_size += 3; // 2 semicolons, 1 newline
                
                auction_id = cur_auction->aid;
                // msg_buf_size += intlen(auction_id);
                msg_buf_size += 50;
                item_name = cur_auction->item_name;
                msg_buf_size += strlen(item_name);
                winning_bid = cur_auction->highest_bid;
                // msg_buf_size += intlen(winning_bid);
                msg_buf_size += 50;

                reply_msg = realloc(reply_msg, msg_buf_size);              
                sprintf(reply_msg, "%d;%s;%d\n", auction_id, item_name, winning_bid);  
            }

            // respond USRWINS to client
            petr_header rep_header;
            rep_header.msg_len = strlen(reply_msg);
            rep_header.msg_type = USRWINS;
            wr_msg(requesting_user->fd, &rep_header, reply_msg);

            //free reply_msg
            free(reply_msg);
        }

        // *** USRSALES ***
        else if (msg_type == 0x34) {
            User_t* requesting_user = FindUserFD(client_fd);
            List_t* sold_auctions_list = requesting_user->sold_auctions_list;
            char* reply_msg = NULL;
            int msg_buf_size = 1;
            petr_header rep_header;

            // if requesting user has not sold anything, msg_len = 0
            if (sold_auctions_list->length == 0) {
                rep_header.msg_len = 0;
            }
            else {
                // format: <auction_id>;<item_name>;<winning_user>;<winning_bid>\n...
                // if auction has no winner, <winning_user> and <winning_bid> = "None"
                node_t* cur_node = sold_auctions_list->head;
                Auction_t* cur_auction = NULL;
                int auction_id;
                char* item_name = NULL;
                char* winning_user = NULL;
                int winning_bid;
                while (cur_node != NULL) {
                    cur_auction = cur_node->value;

                    msg_buf_size += 4; // 3 semicolons, 1 newline

                    auction_id = cur_auction->aid;
                    // msg_buf_size += intlen(auction_id);
                    msg_buf_size += 50;
                    item_name = cur_auction->item_name;
                    msg_buf_size += strlen(item_name);

                    if (cur_auction->highest_bidder == NULL) {  // No winner
                        // winning_user = "None"
                        // winning_bid = "None"
                        // strlen("None") == 4, have 2 strings
                        msg_buf_size += 8; 
                    }
                    else {
                        winning_user = cur_auction->highest_bidder; // winning_user
                        msg_buf_size += strlen(winning_user);
                        winning_bid = cur_auction->highest_bid; // winning_bid
                        // msg_buf_size += intlen(winning_bid);
                        msg_buf_size += 50;
                    }

                    reply_msg = realloc(reply_msg, msg_buf_size);
                    printf(reply_msg, "%d;%s;%s;%d\n", auction_id, item_name, winning_user, winning_bid);

                    cur_node = cur_node->next;
                }
            }

            // send rep to client_fd
            rep_header.msg_len = strlen(reply_msg);
            rep_header.msg_type = USRSALES;
            wr_msg(requesting_user->fd, &rep_header, reply_msg);
            //free reply_msg
            free(reply_msg);
        }

        // *** USRBLNC ***
        else if (msg_type == 0x35) {//USRBLNC
            //returns current users balance, The users balance is total sole - total bought
            User_t* requesting_user = FindUserFD(client_fd);

            char number[20];
            sprintf(number, "%d", requesting_user->balance);
            
            petr_header usrblnc_rep_header;
            usrblnc_rep_header.msg_len = strlen(number);
            usrblnc_rep_header.msg_type = USRBLNC;
            wr_msg(requesting_user->fd, &usrblnc_rep_header, number);
        }
        //free(job_msg->msg);
        //free(job_msg);
    }

    
    return NULL;
}

void* Yellow_Thread(void* vargp) {
    int is_realtime_mode = ((YTA_t*)vargp)->is_realtime_mode;
    int tick_len_seconds = ((YTA_t*)vargp)->tick_len_seconds;
    node_t* cur_node = NULL;
    Auction_t* cur_auction = NULL;

    // REALTIME MODE
    if (is_realtime_mode) {
        while(1){
            
            sem_wait(&auctions_write_lock);
            // Iterate over all auctions, decrease their remaining_ticks, if 0 enqueue ANCLOSED job
            cur_node = Auctions->head;
            while (cur_node != NULL) {
                cur_auction = cur_node->value;
                (cur_auction->ticks_remain)--;
                if (cur_auction->ticks_remain == 0) {
                    Msg_t* job_msg = malloc(sizeof(Msg_t));
                    job_msg->client_fd = cur_auction->aid;
                    job_msg->msg_type = ANCLOSED;
                    job_msg->msg_len = 0;
                    job_msg->msg = NULL;
                    sbuf_insert(&Job_Queue, job_msg);
                }
                cur_node = cur_node->next;
            }
            sem_post(&auctions_write_lock);

            sleep(tick_len_seconds);
        }
    }

    // DEBUG MODE
    else {
        while(1){
            
            char tick;
            scanf("%c", &tick); // Only works with 1 single \n
            printf("tick\n"); 
            sem_wait(&auctions_write_lock);
            // Iterate over all auctions, decrease their remaining_ticks, if 0 enqueue ANCLOSED job
            cur_node = Auctions->head;
            while (cur_node != NULL) {
                printf("decrease 1\n");
                cur_auction = cur_node->value;
                (cur_auction->ticks_remain)--;
                if (cur_auction->ticks_remain == 0) {
                    Msg_t* job_msg = malloc(sizeof(Msg_t));
                    job_msg->client_fd = cur_auction->aid;
                    job_msg->msg_type = ANCLOSED;
                    job_msg->msg_len = 0;
                    job_msg->msg = NULL;
                    sbuf_insert(&Job_Queue, job_msg);
                }

                cur_node = cur_node->next;
            }
            sem_post(&auctions_write_lock);
        }
    }
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

void delete_file_auction(node_t* auction){
    Auction_t* cur_auction = auction->value;
    free(cur_auction->item_name);

    deleteList(cur_auction->watchers_list);

    free(auction);
    return;
       

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
    new_account->won_auctions_list = NULL;
    new_account->fd = fd;
    new_account->username = username;
    new_account->password = password;
    new_account->isOnline = 1;
    new_account->balance = 0;

    List_t* won_auctions = malloc(sizeof(List_t));
    won_auctions->head = NULL;
    won_auctions->length = 0;
    won_auctions->comparator = NULL;
    new_account->won_auctions_list = won_auctions;

    List_t* sold_auctions = malloc(sizeof(List_t));
    sold_auctions->head = NULL;
    sold_auctions->length = 0;
    sold_auctions->comparator = NULL;
    new_account->sold_auctions_list = sold_auctions;

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

User_t* FindUserFD(int client_fd) {
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

User_t* FindUserName(char* username) {
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

        if (strcmp(cur_val->username, username) == 0) {
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


User_t* FindWatcher(Auction_t* auction, int client_fd) {
    node_t* cur_node = auction->watchers_list->head;
    User_t* cur_val;
    while (cur_node != NULL) {  // Find the username that requested
        printf("Finding User\n");
        cur_val = (User_t*)(cur_node->value);

        if (cur_val->isOnline == 1 && client_fd == cur_val->fd) {
            return cur_val;
        }

        cur_node = cur_node->next;
    }
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

int GetWatchersCount(List_t* watchers_list) {
    node_t* cur_watcher_node = watchers_list->head;
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

// int intlen(int n) {
//     if (n == 0) {
//         return 1;
//     }
//     else {
//         return floor(log10(abs(n))) + 1;
//     }
// }

                


#endif  // SERVER_H
