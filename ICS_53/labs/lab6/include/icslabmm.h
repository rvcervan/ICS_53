/**
 * === DO NOT MODIFY THIS FILE ===
 * If you need some other prototypes or constants in a header, please put them in
 * helpers.h
 *
 * When we grade, we will be replacing this file with our own copy.
 * You have been warned.
 * === DO NOT MODIFY THIS FILE ===
 */
#ifndef ICSLABMM_H
#define ICSLABMM_H
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define PAGE_SIZE 4096

typedef uint64_t u64;

extern bool verbose;
extern void *implicit_list_head;
#ifdef NEXT_FIT
extern void *implicit_list_next; // the next pointer in nextfit
#endif 

void *icslab_malloc(size_t size);
int icslab_free(void *ptr);

void icslab_init();
void *find_and_allocate(size_t size);
void coalesce(void *header);
int extend_heap(int npages);

void icslab_mem_init(bool verbose);
void icslab_mem_fini();
void *icslab_get_brk();
void *icslab_inc_brk();

void icslab_list_print();
int icslab_payload_print(void *payload);
int icslab_header_print(void *header);

#endif
