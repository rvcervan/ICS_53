// DO NOT MODIFY THIS FILE
// Any additions should be placed in helpers2.h

#ifndef HW_2_H
#define HW_2_H

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "linkedlist.h"
#include "helpers2.h"
#include "constants2.h"

typedef struct {
    char ASCII;
    char* sequence;
} mapping_t;

// Part 1 Functions to implement
void createSecretKey(char* keyphrase, char* keyArray);
char morseToKey(char* mcmsg, char* FMCtoKey, char* keyArray);

// Part 2 Functions to implement
void mapping_tPrinter(void* val_ref);
int mapping_tComparator(void* lhs, void* rhs);
void mapping_tDeleter(void* val_ref);
void DestroyList(list_t* list);
mapping_t* CreateMapping(char* line);
list_t* GenerateFMCMappings(char* filename);
mapping_t* FindASCII(char token, list_t* list);

// Part 3 Functions to implement
int createMorse(list_t* mapping, char **mc_ptr);
int encrypt(list_t* mapping, char* FMCtoKey , char* phrase, FILE *outstream);

// Extra Credit
mapping_t* MatchKeyword(char token, int token_length, list_t* list);
int fromMorse(char* mcmsg, list_t* mapping, FILE* outstream);
int decrypt(list_t* mapping, char* FMCtoKey , char* phrase, FILE *outstream);

#endif
