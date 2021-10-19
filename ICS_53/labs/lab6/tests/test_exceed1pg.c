#include "debug.h"
#include "helpers.h"
#include "icslabmm.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  // Initialize the custom allocator
  icslab_mem_init(false);

  // Print out title for first test
  printf("=== Test: block allocations which in total exceed 1 page ===\n\n");
 
  printf("\n\n=== Before any malloc call ===\n\n");
  icslab_list_print();
  press_to_cont();

  icslab_malloc(4000);
  
  printf("\n\n=== After allocation for 4000 bytes ===\n\n"); 
  icslab_list_print();
  press_to_cont();

  icslab_malloc(100);          

  printf("\n\n=== After allocation for 100 bytes ===\n\n");
  icslab_list_print();
  
  icslab_mem_fini();

  return EXIT_SUCCESS;
}
