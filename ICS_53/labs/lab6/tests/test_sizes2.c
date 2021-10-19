#include "debug.h"
#include "helpers.h"
#include "icslabmm.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct node {
    void* value;
    struct node* next;
} node_t;

typedef struct list {
    node_t* head;
    int length;
    int (*comparator)(void*, void*);
} List_t;




int main(int argc, char *argv[]) {
  // Initialize the custom allocator
  icslab_mem_init(true);

  // Print out title for first test
  printf("=== Test: Allocate for different data sizes) ===\n\n");
  
  void *p_char = icslab_malloc(50);  
  printf("\n\n=== After allocating for an array of 50 char ===\n\n");
  icslab_list_print();
  press_to_cont();

  void *p_List_t = icslab_malloc(sizeof(List_t));  
  printf("\n\n=== After allocating for a List_t struct (%ld bytes) ===\n\n", sizeof(List_t));
  icslab_list_print();
  press_to_cont();

  printf("\n\n=== Free array of 50 char ===\n\n");
  icslab_free(p_char);
  printf("\n\n=== Free List_t ===\n\n");
  icslab_free(p_List_t);

  printf("\n\n=== After freeing all blocks ===\n\n");
  icslab_list_print();

  icslab_mem_fini();

  return EXIT_SUCCESS;
}
