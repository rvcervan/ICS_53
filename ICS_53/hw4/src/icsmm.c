#include "icsmm.h"
#include "debug.h"
#include "helpers.h"
#include <stdio.h>
#include <stdlib.h>

/*
 * This is your implementation of malloc. It acquires uninitialized memory from  
 * ics_inc_brk() that is 16-byte aligned, as needed. <----------------------------------------- byte alignment
 *
 * @param size The number of bytes requested to be allocated.
 *
 * @return If successful, the pointer to a valid region of memory of at least the
 * requested size is returned. Otherwise, NULL is returned and errno is set to 
 * ENOMEM - representing failure to allocate space for the request.
 * 
 * If size is 0, then NULL is returned and errno is set to EINVAL - representing
 * an invalid request.
 */

 /*
    My Notes:
    LIFO
    5 different free lists to store free blocks into aka buckets
        * bucket 0 with max size 64 holds blocks in range of (0, 64]
        * bucket 1 with max size 128 holds blocks in range of (64, 128], and so on.
    supports the First Fit Placement policy. The first block in the free list which can satisfy the malloc request is used.
    When splitting blocks your allocator MUST NOT create splinters.
        * to avoid this your allocator must "over" allocate the amount of memory requested by the malloc call so that these 
          small useless blocks are not inserted into the free list, but remain with the allocated block.
 */
int first_call = 0; //if 0 when ics_malloc is called, then it is the first malloc call
void* brk_holder = NULL; //holds the first page's beginning breakpoint
int page_size = 4080;
void *ics_malloc(size_t size) {
    //currently no space in heap, after first call to ics_malloc get 1 page for heap with ics_inc_brk()
    if(size == 0){ 
    errno = EINVAL;
    return NULL;
    }

    if(first_call == 0){//get first page and prep it so that we can malloc stuff into it.
        void* start = ics_inc_brk();//now we have page 1, size of 4096 bytes
        
        
        //set the prologue
        ((ics_footer*)start)->fid = FOOTER_MAGIC;
        ((ics_footer*)start)->requested_size = 0;
        ((ics_footer*)start)->block_size = 0;
        ((ics_footer*)start)->block_size = ((ics_footer*)start)->block_size | 1;//set the prologue, bitwise or lsb by 1 to make it alloc'd

        //set the epilogue, 8 bytes
        void* last = ics_get_brk();
        last = last-8;
        ((ics_header*)last)->block_size = 0 | 1;
        ((ics_header*)last)->hid = HEADER_MAGIC;

        //set first free header
        start = start+8;//move up by 8 to set header with its id and size
        
        ((ics_free_header*)start)->header.hid = HEADER_MAGIC;
        ((ics_free_header*)start)->header.block_size = 4080;
        

        //start = start+8; //move up by 8 to set next pointer <----- dont have to do this
        ((ics_free_header*)start)->next = NULL;
        
        //start = start+8; //move up by 8 to set prev pointer <----- dont have to do this
        ((ics_free_header*)start)->prev = NULL;

        brk_holder = start;

        //set up buckets
        
        int i;
        for(i = 0; i < 5; ++i){
            if(((ics_free_header*)start)->header.block_size <= (seg_buckets+i)->max_size){
                (seg_buckets+i)->freelist_head = start;
                break;
            }
        }
        
        //set footer of page
        last = last-8;
        ((ics_footer*)last)->fid = FOOTER_MAGIC;
        ((ics_footer*)last)->block_size = 4080;
        
        //done preping page.
        


    }

    //figure out how to make the size be aligned.
    size_t padded_size = roundUp(size);
 
    //printf("padded_size is %ld\n", padded_size); 
    
    int i;
    //search the free list to find space to place the block.
    
    void* return_address = NULL; //holds address to return

    //if first first malloc do
    //printf("first call var is %d\n", first_call);
    
    again:

    if(first_call == 0){//it is the first malloc call
        //page size is 4080
        //brk_holder holds start of the page
        //if 4080-padded_size < 32. get new page and do calculation again. repeat until it fits without splinter or you run out of pages.
        //printf("This shouldn't print twice\n"); 
        
        int dif = page_size - (padded_size+16);
        //printf("%d\n", dif); 
        if((dif < 32 && dif > 0) || dif < 0){// creates splinter get new page or not enough space get new page
            //printf("increase the page, malloc size is too big\n");
            void* new_p = ics_inc_brk();

            //modify page start free header with new size
            void* start = brk_holder;

            int sss = ((ics_free_header*)start)->header.block_size;

            ((ics_free_header*)start)->header.block_size = sss + 4096;
            page_size = page_size + 4096;

            //set the new epilogue
            void* last = ics_get_brk();
            last = last-8;
            ((ics_header*)last)->block_size = 1;
            
            //set new page footer
            ((ics_footer*)last)->block_size = ((ics_free_header*)start)->header.block_size;
            ((ics_footer*)last)->fid = FOOTER_MAGIC;

            //empty the buckets
            (seg_buckets+0)->freelist_head = NULL;
            (seg_buckets+1)->freelist_head = NULL;
            (seg_buckets+2)->freelist_head = NULL;
            (seg_buckets+3)->freelist_head = NULL;
            (seg_buckets+4)->freelist_head = NULL;

            //replace the buckets
            int i;
            for(i = 0; i < 5; ++i){
                if(((ics_free_header*)start)->header.block_size <= (seg_buckets+i)->max_size){
                    ((ics_free_header*)start)->prev = NULL;
                    (seg_buckets+i)->freelist_head = start;
                    break;
                }
            }
   

            //try the alloc again
            goto again;

        }
        else{
            void* mal = brk_holder;
            //set header
            //printf("%d is the free block size\n", ((ics_free_header*)mal)->header.block_size);

            ((ics_header*)mal)->hid = HEADER_MAGIC;
            ((ics_header*)mal)->block_size = padded_size+16+1;
            return_address = mal+8;
            mal = mal+8;
            
            //increase pointer to address of footer;
            mal = mal+padded_size;
            //printf("%ld is the padded size\n", padded_size);

            //set footer
            ((ics_footer*)mal)->fid = FOOTER_MAGIC;

            ((ics_footer*)mal)->requested_size = size;
            ((ics_footer*)mal)->block_size = padded_size+16+1;

            //set new free header
            mal = mal+8;
            ((ics_free_header*)mal)->header.block_size = page_size-(padded_size+16);
            ((ics_free_header*)mal)->header.hid = HEADER_MAGIC;

            //Add header back to bucket.
            (seg_buckets+0)->freelist_head = NULL;
            (seg_buckets+1)->freelist_head = NULL;
            (seg_buckets+2)->freelist_head = NULL;
            (seg_buckets+3)->freelist_head = NULL;
            (seg_buckets+4)->freelist_head = NULL;

            //set next pointer
            ((ics_free_header*)mal)->next = NULL;
            
            //set prev pointer
            ((ics_free_header*)mal)->prev = NULL;

            
            int i;
            for(i = 0; i < 5; ++i){//iterate through buckets to find appropriate size
                if(((ics_free_header*)mal)->header.block_size <= (seg_buckets+i)->max_size){
                    (seg_buckets+i)->freelist_head = mal;
                    break;
                }
            }

            //adjust page footer
            void* mal_last = ics_get_brk();
            mal_last = mal_last-16;
            ((ics_footer*)mal_last)->requested_size = 0;
            ((ics_footer*)mal_last)->block_size = ((ics_free_header*)mal)->header.block_size;
            ((ics_footer*)mal_last)->fid = FOOTER_MAGIC;

            


        }

        first_call = 1;
    }
    else{// it is not the first call
        //printf("requested size is %ld-------------------------------------------------------------------\n", size);
        int i;
        for(i = 0; i < 5; ++i){
            if((padded_size+16) <= (seg_buckets+i)->max_size){//check if block size meets bucket conditions
                
                void* head = (seg_buckets+i)->freelist_head;
                //printf("%p is in head\n", head);
                if(head == NULL){
                    //printf("head is null\n");
                    continue;
                }
              
                //printf("made it pass the if\n");
                while(head){
                    int free_block_size = ((ics_free_header*)head)->header.block_size;
                    int dif = free_block_size - (padded_size+16);
                    //printf("made it pass the two ints\n");
                    if(dif > 0 && dif < 32){//over allocate to prevent splinter
                        padded_size = free_block_size - 16;
                        dif = 0;
                    }


                    //printf("The padded size is %ld and the free block size is %d\n", padded_size, free_block_size);
                    if(dif >= 32 || dif == 0){ //the free block is suitable. remove from list and do work on it.
                        
                        /////////////////////////////////////////////
                        //if the next and free pointer are null, then there is only one node in the list
                        //make that bucket point to null and re-place the new node.
                        //iterate through the buckets until you find the max_size that matches the nodes old size.
                        //make point to null.

                        if( ((ics_free_header*)head)->next == NULL && ((ics_free_header*)head)->prev == NULL){
                            //printf("next = null, prev = null\n");
                            int i;
                            for(i = 0; i < 5; ++i){
                                if( ((ics_free_header*)head)->header.block_size <= (seg_buckets+i)->max_size){
                                    (seg_buckets+i)->freelist_head = NULL;
                                    break;
                                }
                    
                            }


                        }
                        //if the next is null but prev is not then it is in the last of the list.
                        //save the node you are going to move and make the prev node's next point to null
                        //replace the node
                        else if( ((ics_free_header*)head)->next == NULL && ((ics_free_header*)head)->prev != NULL){
                            //printf("next = null, prev != null\n");
                            void* prev = ((ics_free_header*)head)->prev;
                            ((ics_free_header*)prev)->next = NULL;

                        }

                        //if the next is not null and the prev is null, the node is in the front of a linked list
                        //get the nodes block size and iterate through the buckets to find the appropriate bucket
                        //make the freelist_head point to the nodes next
                        //replace the modded node.
                        else if( ((ics_free_header*)head)->next != NULL && ((ics_free_header*)head)->prev == NULL){
                            //printf("next != null, prev == null\n");
                            int i;
                            for(i = 0; i < 5; ++i){
                                if( ((ics_free_header*)head)->header.block_size <= (seg_buckets+i)->max_size){
                                    void* temp = ((ics_free_header*)head)->next;
                                    ((ics_free_header*)temp)->prev = NULL;
                                    (seg_buckets+i)->freelist_head = ((ics_free_header*)head)->next;
                                    break;

                                }
                            }

                        }

                        //if next and prev are neither null, then make the prev's next point to currents next;
                        //make next's prev point to current's prev
                        //replace the node
                        else if( ((ics_free_header*)head)->next != NULL && ((ics_free_header*)head)->prev != NULL){
                            //printf("next != null, prev != null\n");
                            void* next = ((ics_free_header*)head)->next;
                            void* prev = ((ics_free_header*)head)->prev;

                            ((ics_free_header*)next)->prev = prev;
                            ((ics_free_header*)prev)->next = next;

                        }
                        else{
                            //printf("Error: something happen in the new page section\n");
                            return NULL; //some error occured
                        }
                        /////////////////////////////////////////////

                        
                        //head points to free header address// change it to malloced header
                        ((ics_header*)head)->hid = HEADER_MAGIC;
                        ((ics_header*)head)->block_size = padded_size + 16 + 1;

                        //move past payload to set footer;
                        head = head+8;
                        return_address = head;
                        head = head+padded_size;

                        //set alloced footer
                        ((ics_footer*)head)->fid = FOOTER_MAGIC;
                        ((ics_footer*)head)->block_size = padded_size+16+1;
                        ((ics_footer*)head)->requested_size = size;
                        head = head+8;

                        
                        //if dif is 0 then there is no free block to allocate into the buckets
                        if(dif == 0){
                            //dont create new header
                            return return_address;
                        }

                        //set new free header
                        ((ics_free_header*)head)->header.block_size = dif;
                        ((ics_free_header*)head)->header.hid = HEADER_MAGIC;

                        //set next;
                        ((ics_free_header*)head)->next = NULL;

                        //set prev
                        ((ics_free_header*)head)->prev = NULL;

                        //place back in bucket;
                        int i;
                        for(i = 0; i < 5; ++i){//iterate through buckets to find appropriate size
                            if(((ics_free_header*)head)->header.block_size <= (seg_buckets+i)->max_size){
                                if( (seg_buckets+i)->freelist_head != NULL){//means that there is a node in the front
                                    ((ics_free_header*)head)->next = (seg_buckets+i)->freelist_head;
                                    ((ics_free_header*)head)->prev = NULL;
                                    ((ics_free_header*)(seg_buckets+i)->freelist_head)->prev = head;
                                    (seg_buckets+i)->freelist_head = head;

                                }
                                else{
                                    (seg_buckets+i)->freelist_head = head;
                                }
                                break;
                            }
                        }
                        //printf("Am I at the free footer to modify for the %d\n", dif);
                        // modify free footer
                        head = head+8;
                        head = head+(dif-16); // at free footer?

                        ((ics_footer*)head)->block_size = dif;
                        ((ics_footer*)head)->fid = FOOTER_MAGIC;

                        
                        return return_address;

                    }

                    head = ((ics_free_header*)head)->next;
                }

            }

        }
        //if the for loop exits, then that means there was no space on the page to malloc space. Increase the page.
        //printf("%ld is the size that needs to get malloced but couldn't because it was too big\n", size);
        void* new_p = ics_inc_brk();

        //check the previous footer and see if it is malloced or free. 
        //if it is free combine that block with the page to make 1 bigger block. So just add the footer
        //if it is not free, then make the new page a free block. So make free_header and footer.
        void* check = new_p - 16;
        int bit = ((ics_footer*)check)->block_size & 1;
        int size = ((ics_footer*)check)->block_size;
        if(bit == 1){
            //footer is allocated, make header to replace epilogue
            //printf("footer is allocated make new header on the beginning of new page, where the old epilogue is\n");
            check = check+8;//at epilogue

            //create new header
            ((ics_free_header*)check)->header.block_size = 4096;
            ((ics_free_header*)check)->header.hid = HEADER_MAGIC;
            ((ics_free_header*)check)->next = NULL;
            ((ics_free_header*)check)->prev = NULL;

            void* last = ics_get_brk();

            //set the epilogue
            last = last - 8;
            ((ics_header*)last)->block_size = 1;

            //set the free footer
            last = last-8;
            ((ics_footer*)last)->block_size = 4096;
            ((ics_footer*)last)->fid = FOOTER_MAGIC;

            //place the node in the buckets list.
            int i;
            for(i = 0; i < 5; ++i){
                if( ((ics_free_header*)check)->header.block_size <= (seg_buckets+i)->max_size){
                    if( (seg_buckets+i)->freelist_head != NULL){
                        ((ics_free_header*)check)->next = (seg_buckets+i)->freelist_head;
                        ((ics_free_header*)check)->prev = NULL;
                        ((ics_free_header*)(seg_buckets+i)->freelist_head)->prev = check;
                        (seg_buckets+i)->freelist_head = check;
                    }
                    else{
                        (seg_buckets+i)->freelist_head = check;
                    }
                    break;
                
                }
            }
            goto again;

        }
        else{
            //footer is free, get size and make the new page footer 4096 + footer size
            check = check - (size-8);// go down to free header

            //printf("%d is the size of the free block\n", ((ics_free_header*)check)->header.block_size);

            //if the next and free pointer are null, then there is only one node in the list
            //make that bucket point to null and re-place the new node.
            //iterate through the buckets until you find the max_size that matches the nodes old size.
            //make point to null.

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
                return NULL; //some error occured
            }

            ((ics_free_header*)check)->header.block_size = 4096 + size;
            ((ics_free_header*)check)->header.hid = HEADER_MAGIC;
            //replace the node back into the list;
            ((ics_free_header*)check)->next = NULL;
            ((ics_free_header*)check)->prev = NULL;

            int i;
            for(i = 0; i < 5; ++i){
                if( ((ics_free_header*)check)->header.block_size <= (seg_buckets+i)->max_size){
                    if( (seg_buckets+i)->freelist_head != NULL){
                        ((ics_free_header*)check)->next = (seg_buckets+i)->freelist_head;
                        ((ics_free_header*)(seg_buckets+i)->freelist_head)->prev = check;
                        (seg_buckets+i)->freelist_head = check;
                    }
                    else{
                        (seg_buckets+i)->freelist_head = check;
                    }
                    break;
                
                }
            }

            //set the epilogue of the new page
            void* last = ics_get_brk();
            last = last-8;
            ((ics_header*)last)->block_size = 1;
            ((ics_header*)last)->hid = HEADER_MAGIC;

            //set the new freeblocks footer
            last = last-8;
            ((ics_footer*)last)->block_size = 4096 + size;
            ((ics_footer*)last)->fid = FOOTER_MAGIC;

            //try the alloc again
            goto again;
        }
        

    }


    //since page is alligned, set prologue to align header and epiloge at end of page with a footer right below it,
    //then get next and prev pointers and have freeList_head point to header to start the explicit free list.

    //now satisfy the ics_malloc call
    return return_address;
}

/*
 * Marks a dynamically allocated block as no longer in use and coalesces with 
 * adjacent free blocks. Adds the block to the appropriate bucket according
 * to the block placement policy.
 *
 * @param ptr Address of dynamically allocated memory returned by the function
 * ics_malloc.
 * 
 * @return 0 upon success, -1 if error and set errno accordingly.
 * 
 * If the address of the memory being freed is not valid, this function sets errno
 * to EINVAL. To determine if a ptr is not valid, (i) the header and footer are in
 * the managed  heap space, (ii) check the hid field of the ptr's header for
 * 0x100decafbee5 (iii) check the fid field of the ptr's footer for 0x0a011dab,
 * (iv) check that the block_size in the ptr's header and footer are equal, and (v) 
 * the allocated bit is set in both ptr's header and footer. 
 */

 /*
    My Notes:
    immediate coalescing upon free()
    Each block uses boundary tags which is a single memory row of 64-bits in this case
 */
int ics_free(void *ptr) { 
    //make the ptr into a free header

    //check if the free is in bounds of the heap.
    void* start = brk_holder-8;
    void* last = ics_get_brk();

    

    if(ptr < start || ptr > last){
        errno = EINVAL;
        return -1;

    }
/*
    void* h_block = ptr-8;
    int h_alloc = ((ics_header*)h_block)->block_size & 1;
    void* f_block = ptr + ( ((ics_header*)ptr)->block_size - 8);
    int f_alloc = ((ics_footer*)f_block)->block_size & 1;
    if(f_block < start || f_block > last){
        errno = EINVAL;
        return -1;
    }

    if( ((ics_header*)h_block)->hid != HEADER_MAGIC){
        errno = EINVAL;
        return -1;
    }

    if( ((ics_footer*)f_block)->fid != FOOTER_MAGIC){
        errno = EINVAL;
        return -1;
    }

    if( ((ics_header*)h_block)->block_size != ((ics_header*)f_block)->block_size){
        errno = EINVAL;
        return -1;
    }

    if( h_alloc != 1 || f_alloc != 1){
        errno = EINVAL;
        return -1;
    }
*/    
    
    

    //check the four cases to coalesce
    ptr = ptr-16;
    int footer_bit = ((ics_footer*)ptr)->block_size & 1;//get the footer bit;
    int prev_size = ((ics_footer*)ptr)->block_size; //size of the prev block
    //printf("%d is prev blocks size\n", prev_size);
    

    void* temp = ptr+8;
    temp = temp + (((ics_header*)temp)->block_size-1);

    int header_bit = ((ics_header*)temp)->block_size & 1;//get the header bit
    int next_size = ((ics_header*)temp)->block_size; //size of the next block;
    

    ptr = ptr+8;
    int current_size = ((ics_header*)ptr)->block_size - 1;

    //printf("%d is current blocks size\n", current_size);
    //printf("%d is next blocks size\n", next_size);



    if(header_bit == 1 && footer_bit == 1){
        //there is no free blocks on either side
        //make the new free block with no coalesce
        void* new_node = ptr;

        ((ics_free_header*)ptr)->header.block_size--;
        ((ics_free_header*)ptr)->header.hid = HEADER_MAGIC;
        ((ics_free_header*)ptr)->next = NULL;
        ((ics_free_header*)ptr)->prev = NULL;

        ptr = ptr + ((ics_free_header*)ptr)->header.block_size - 8;//go to footer

        ((ics_footer*)ptr)->block_size--;
        ((ics_footer*)ptr)->fid = FOOTER_MAGIC;

        //place back to buckets
        int i;
        for(i = 0; i < 5; ++i){
            if( ((ics_free_header*)new_node)->header.block_size <= (seg_buckets+i)->max_size){
                if( (seg_buckets+i)->freelist_head != NULL){
                    ((ics_free_header*)new_node)->next = (seg_buckets+i)->freelist_head;
                    ((ics_free_header*)new_node)->prev = NULL;
                    ((ics_free_header*)(seg_buckets+i)->freelist_head)->prev = new_node;
                    (seg_buckets+i)->freelist_head = new_node;
                }
                else{
                    ((ics_free_header*)new_node)->next = NULL;
                    ((ics_free_header*)new_node)->prev = NULL;
                    (seg_buckets+i)->freelist_head = new_node;
                }
                break;
                
            }
        }
    }
    else if(footer_bit == 1 && header_bit == 0){
        //the block to the right of current block is also free coalesce the two

        //first remove the free block to the right from the bucket list
        void* temp = ptr + current_size;
        remove_node(temp);

        //update the current blocks header to free header with new size
        void* new_node = ptr;

        ((ics_free_header*)new_node)->header.block_size = current_size + next_size;
        ((ics_free_header*)new_node)->header.hid = HEADER_MAGIC;
        ((ics_free_header*)new_node)->next = NULL;
        ((ics_free_header*)new_node)->prev = NULL;

        //go to the next blocks footer to update the size
        ptr = ptr + ((current_size + next_size) - 8);
        //printf("current_size block is %d and next_size block is %d\n", current_size, next_size);
        ((ics_footer*)ptr)->block_size = current_size + next_size;
        ((ics_footer*)ptr)->fid = FOOTER_MAGIC;
        
        //place back to buckets
        int i;
        for(i = 0; i < 5; ++i){
            if( ((ics_free_header*)new_node)->header.block_size <= (seg_buckets+i)->max_size){
                if( (seg_buckets+i)->freelist_head != NULL){
                    ((ics_free_header*)new_node)->next = (seg_buckets+i)->freelist_head;
                    ((ics_free_header*)new_node)->prev = NULL;
                    ((ics_free_header*)(seg_buckets+i)->freelist_head)->prev = new_node;
                    (seg_buckets+i)->freelist_head = new_node;
                }
                else{
                    ((ics_free_header*)new_node)->next = NULL;
                    ((ics_free_header*)new_node)->prev = NULL;
                    (seg_buckets+i)->freelist_head = new_node;
                }
                break;
                
            }
        }
    }
    else if(footer_bit == 0 && header_bit == 1){
        //prev block is free, coalesce the two
        
        //first remove the free block to the left from the bucket list
        void* temp = ptr - prev_size;
        remove_node(temp);

        //update the prev blocks head to new free header
        void* new_node = ptr - prev_size;
        
        ((ics_free_header*)new_node)->header.block_size = current_size + prev_size;
        ((ics_free_header*)new_node)->header.hid = HEADER_MAGIC;
        ((ics_free_header*)new_node)->next = NULL;
        ((ics_free_header*)new_node)->prev = NULL;

        //go to current blocks footer to update the size
        ptr = ptr + (current_size - 8);

        ((ics_footer*)ptr)->block_size = current_size + prev_size;
        ((ics_footer*)ptr)->fid = FOOTER_MAGIC;

        //place back to buckets
        int i;
        for(i = 0; i < 5; ++i){
            if( ((ics_free_header*)new_node)->header.block_size <= (seg_buckets+i)->max_size){
                if( (seg_buckets+i)->freelist_head != NULL){
                    ((ics_free_header*)new_node)->next = (seg_buckets+i)->freelist_head;
                    ((ics_free_header*)new_node)->prev = NULL;
                    ((ics_free_header*)(seg_buckets+i)->freelist_head)->prev = new_node;
                    (seg_buckets+i)->freelist_head = new_node;
                }
                else{
                    ((ics_free_header*)new_node)->next = NULL;
                    ((ics_free_header*)new_node)->prev = NULL;
                    (seg_buckets+i)->freelist_head = new_node;
                }
                break;
                
            }
        }

        
    }
    else if(footer_bit == 0 && header_bit == 0){
        //blocks on both sides are free coalesce both into current

        //remove the prev block from the free list
        void* temp = ptr - prev_size;
        remove_node(temp);

        //remove the next block from the free list
        temp = ptr + current_size;
        remove_node(temp);
        
        //update the prev blocks header to new free header
        void* new_node = ptr - prev_size;

        ((ics_free_header*)new_node)->header.block_size = current_size + prev_size + next_size;
        ((ics_free_header*)new_node)->header.hid = HEADER_MAGIC;
        ((ics_free_header*)new_node)->next = NULL;
        ((ics_free_header*)new_node)->prev = NULL;

        //go to next blocks footer to update the size
        ptr = ptr + ((current_size+next_size)-8);

        ((ics_footer*)ptr)->block_size = current_size + prev_size + next_size;
        ((ics_footer*)ptr)->fid = FOOTER_MAGIC; 

        //place back to buckets
        int i;
        for(i = 0; i < 5; ++i){
            if( ((ics_free_header*)new_node)->header.block_size <= (seg_buckets+i)->max_size){
                if( (seg_buckets+i)->freelist_head != NULL){
                    ((ics_free_header*)new_node)->next = (seg_buckets+i)->freelist_head;
                    ((ics_free_header*)new_node)->prev = NULL;
                    ((ics_free_header*)(seg_buckets+i)->freelist_head)->prev = new_node;
                    (seg_buckets+i)->freelist_head = new_node;
                }
                else{
                    ((ics_free_header*)new_node)->next = NULL;
                    ((ics_free_header*)new_node)->prev = NULL;
                    (seg_buckets+i)->freelist_head = new_node;
                }
                break;
                
            }
        }

    }
    

    return 0;

}

/*
 * EXTRA CREDIT!!!
 *
 * Resizes the dynamically allocated memory, pointed to by ptr, to at least size 
 * bytes. Before, attempting to realloc, the current block is coalesced with the free
 * adjacent block with the higher address only.  
 * 
 * If the current block will exactly satisfy the request (without creation of a 
 * splinter), return the current block. 
 * 
 * If reallocation to a larger size, the current (possibly coalesced) block will be
 * used if large enough to satisfy the request. If the current block is not large
 * enough to satisfy the request,  a new block is selected from the free list. All
 * payload bytes of the original block are copied to the new block, if the block was 
 * moved. 
 *
 * If the reallocation size is a reduction in space from the current (possible
 * coalesced) block, resize the block as necessary, creating a new free block if the 
 * free'd space will not produce a splinter.
 *
 * @param ptr Address of the previously allocated memory region.
 * @param size The minimum size to resize the allocated memory to.
 * @return If successful, the pointer to the block of allocated memory is
 * returned. Else, NULL is returned and errno is set appropriately.
 *
 * If there is no memory available ics_malloc will set errno to ENOMEM. 
 *
 * If ics_realloc is called with an invalid pointer, set errno to EINVAL. See ics_free
 * for more details.
 *
 * If ics_realloc is called with a valid pointer and a size of 0, the allocated     
 * block is free'd and return NULL.
 */
void *ics_realloc(void *ptr, size_t size) {
    return NULL;
}
