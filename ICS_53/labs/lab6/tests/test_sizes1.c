#include "debug.h"
#include "helpers.h"
#include "icslabmm.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  // Initialize the custom allocator
  icslab_mem_init(true);

  // Print out title for first test
  printf("=== Test: Allocate for different data sizes) ===\n\n");
  
  icslab_malloc(sizeof(char));  
  printf("\n\n=== After allocating for a single char ===\n\n");
  icslab_list_print();
  press_to_cont();


  icslab_malloc(sizeof(int));  
  printf("\n\n=== After allocating for a single int ===\n\n");
  icslab_list_print();
  press_to_cont();

  icslab_malloc(sizeof(int *));  
  printf("\n\n=== After allocating for an integer pointer ===\n\n");
  icslab_list_print();
  press_to_cont();

  icslab_malloc(sizeof(long long));  
  printf("\n\n=== After allocating for a long long ===\n\n");
  icslab_list_print();
  press_to_cont();

  icslab_malloc(sizeof(double));  
  printf("\n\n=== After allocating for a double ===\n\n");
  icslab_list_print();

  icslab_mem_fini();

  return EXIT_SUCCESS;
}
