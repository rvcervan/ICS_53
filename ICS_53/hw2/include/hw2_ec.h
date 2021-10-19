#ifndef HW_2_EC_H
#define HW_2_EC_H

mapping_t* FindMorseCode(char* token, int token_length, list_t* list);
int fromMorse(char* mcmsg, list_t* mapping, FILE* outstream);
int decrypt(list_t* mapping, char* FMCtoKey , char* phrase, FILE *outstream);

#endif
