#include "debug.h"
#include "icslabmm.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "helpers.h"

int main(int argc, char *argv[]) {
  // Initialize the custom allocator
  icslab_mem_init(false);

  // Print out title for first test
  printf("=== Test: Freeing a block ===\n\n"); 
  // Allocate an integer
  void *p1 = icslab_malloc(48);             // 1st block
  void *p2 = icslab_malloc(60);             // 2nd block
  void *p3 = icslab_malloc(512);            // 3rd block
  void *p4 = icslab_malloc(60);             // 4th block
  void *p5 = icslab_malloc(4);              // 5th block
  void *p6 = icslab_malloc(60);            // 6th block 

  printf("\n\n=== Before calling free  ===\n\n");
  icslab_list_print();
  press_to_cont();

  icslab_free(p2);
  icslab_free(p4);
  icslab_free(p6);

  printf("\n\n=== After calling free ===\n\n");
  icslab_list_print();
  press_to_cont();

  void* p7 = icslab_malloc(60);     

  printf("\n\n=== After malloc-ing again ===\n\n");
  icslab_list_print();

  icslab_mem_fini();

  return EXIT_SUCCESS;
}
