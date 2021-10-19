#include "helpers.h"
#include "debug.h"
#include "icsmm.h"

/* Helper function definitions go here */

size_t roundUp(size_t size){
    
    size_t s = size;
    while(1){
       
       double x = (int)s/16.00;
        
       int trunc = (int)x;
       if(trunc == x){
            return s;
       }
       ++s;
    }
    return -1;
}

int remove_node(void* check){
    
    if( ((ics_free_header*)check)->next == NULL && ((ics_free_header*)check)->prev == NULL){
        //printf("next = null, prev = null\n");
        int i;
        for(i = 0; i < 5; ++i){
            if( ((ics_free_header*)check)->header.block_size <= (seg_buckets+i)->max_size){
                (seg_buckets+i)->freelist_head = NULL;
                break;
            }
                    
        }
        

    }
    //if the next is null but prev is not then it is in the last of the list.
    //save the node you are going to move and make the prev node's next point to null
    //replace the node
    else if( ((ics_free_header*)check)->next == NULL && ((ics_free_header*)check)->prev != NULL){
        //printf("next = null, prev != null\n");
        void* prev = ((ics_free_header*)check)->prev;
        ((ics_free_header*)prev)->next = NULL;
        
    }

    //if the next is not null and the prev is null, the node is in the front of a linked list
    //get the nodes block size and iterate through the buckets to find the appropriate bucket
    //make the freelist_head point to the nodes next
    //replace the modded node.
    else if( ((ics_free_header*)check)->next != NULL && ((ics_free_header*)check)->prev == NULL){
        //printf("next != null, prev == null\n");
        int i;
        for(i = 0; i < 5; ++i){
            if( ((ics_free_header*)check)->header.block_size <= (seg_buckets+i)->max_size){
                void* temp = ((ics_free_header*)check)->next;
                ((ics_free_header*)temp)->prev = NULL; 
                (seg_buckets+i)->freelist_head = ((ics_free_header*)check)->next;
                break;

                }
            }
        
    }

    //if next and prev are neither null, then make the prev's next point to currents next;
    //make next's prev point to current's prev
    //replace the node
    else if( ((ics_free_header*)check)->next != NULL && ((ics_free_header*)check)->prev != NULL){
        //printf("next != null, prev != null\n");
        void* next = ((ics_free_header*)check)->next;
        void* prev = ((ics_free_header*)check)->prev;

        ((ics_free_header*)next)->prev = prev;
        ((ics_free_header*)prev)->next = next;
        
    }
    else{
        //printf("Error: something happen in the new page section\n");
            return -1; //some error occured
    } 
    return 0;
}






