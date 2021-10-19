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
  printf("=== Test: Splitting a block ===\n\n"); 
  // Allocate an integer
  icslab_malloc(48);             // 1st block
  icslab_malloc(128);            // 2nd block
  void *p3 = icslab_malloc(512);            // 3rd block
  icslab_malloc(60);             // 4th block
  icslab_malloc(4);              // 5th block
  icslab_free(p3);

  printf("\n\n=== After calling free  ===\n\n");
  icslab_list_print();
  press_to_cont();

  icslab_malloc(50);     

  printf("\n\n=== After calling malloc ===\n\n");
  icslab_list_print();

  icslab_mem_fini();

  return EXIT_SUCCESS;
}
