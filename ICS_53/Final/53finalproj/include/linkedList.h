#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>


#define INT_MODE 0
#define STR_MODE 1



/*
 * Structre for each node of the linkedList
 *
 * value - a pointer to the data of the node. 
 * next - a pointer to the next node in the list. 
 */
typedef struct node {
    void* value;
    struct node* next;
} node_t;

/*
 * Structure for the base linkedList
 * 
 * head - a pointer to the first node in the list. NULL if length is 0.
 * length - the current length of the linkedList. Must be initialized to 0.
 * comparator - function pointer to linkedList comparator. Must be initialized!
 */
typedef struct list {
    node_t* head;
    int length;
    /* the comparator uses the values of the nodes directly (i.e function has to be type aware) */
    int (*comparator)(void*, void*);
} List_t;

typedef struct User {
    char* username;
    char* password;
    int fd;
    int isOnline;
    int balance;
    List_t* won_auctions_list;// List of WON auctions, must be sorted lexcographically
    List_t* sold_auctions_list; // List of CLOSED auctions CREATED by user
} User_t;

typedef struct Auction {
    int aid;
    char* creator_username;
    int ticks_remain;
    char* item_name;
    int bin_price;
    int highest_bid; 
    char* highest_bidder;
    List_t* watchers_list;
} Auction_t;

typedef struct GreenThreadArgs {
    int client_fd;
    int listen_fd;
} GTA_t;

typedef struct YellowThreadArgs {
    int is_realtime_mode;
    int tick_len_seconds;
} YTA_t;

/* 
 * Each of these functions inserts the reference to the data (valref)
 * into the linkedList list at the specified position
 *
 * @param list pointer to the linkedList struct
 * @param valref pointer to the data to insert into the linkedList
 */
void insertRear(List_t* list, void* valref);
void insertFront(List_t* list, void* valref);
void insertInOrder(List_t* list, void* valref);

/*
 * Each of these functions removes a single linkedList node from
 * the LinkedList at the specfied function position.
 * @param list pointer to the linkedList struct
 * @return a pointer to the removed list node
 */ 
void* removeFront(List_t* list);
void* removeRear(List_t* list);
void* removeByIndex(List_t* list, int n);

/*
Locking and unlocking sem_t should be done before and after using these functions
*/
void* removeByName(List_t* list, char* name);
void* removeByName_NoFree(List_t* list, char* name);
void* removeByAid(List_t* list, int aid);


/* 
 * Free all nodes from the linkedList
 *
 * @param list pointer to the linkedList struct
 */
void deleteList(List_t* list);

/*
 * Traverse the list printing each node in the current order.
 * @param list pointer to the linkedList strut
 * @param mode STR_MODE to print node.value as a string,
 * INT_MODE to print node.value as an int
 */
void printList(List_t* list, char mode);

#endif
