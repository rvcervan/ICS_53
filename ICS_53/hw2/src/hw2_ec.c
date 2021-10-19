// Raul Cervantes
// rvcervan

#include "hw2.h"
#include "hw2_ec.h"
// Extra Credit Functions

mapping_t* FindMorseCode(char* token, int token_length, list_t* list) {
    if(!token || !list){ return NULL; }
    
    node_t *head = list->head;
    mapping_t *node;
    int matches = 0; //if matches == token_length then toke matches and return mapping_t
    while(head != NULL){
        node_t *temp = head->next;
        node = head->data;
        int i;
        for(i = 0; matches < token_length; ++i){
            if(*(token+i) == *((node->sequence)+i)){
                matches++;
            }

            if(matches == token_length){ return node; }

            if(*(token+i) == 'x' || *((node->sequence)+i) == '\0'){
                head = temp;

                continue;
            }
        }

        head = temp;

    }
    return NULL;


}

int fromMorse(char* mcmsg, list_t* mapping, FILE* outstream) {
    //mcmsg is starting address of morse code sequence
    //mapping is the linked list mapping
    //outstream is the output for the message
    if(!mcmsg || !mapping || !outstream){ return -1; }
    int seq_len = 0;
    int array_pos = 0;
    int start = 0;
    mapping_t *node = NULL;
    while(*(mcmsg+array_pos) != '\0'){
        if(*(mcmsg+array_pos) == 'x'){
            node = FindMorseCode((mcmsg+start), seq_len, mapping);
            if(!node){return -1;}
            fprintf(outstream, "%c", node->ASCII);
            seq_len = 0;
            if(*(mcmsg+(array_pos+1)) == 'x'){
                fprintf(outstream, "%c", ' ');
                array_pos++;
                start = array_pos + 1;
            }
            else{
                start = array_pos+1;
            }
            
        }
        seq_len++;
        array_pos++;
    }
    return 0;

}

int decrypt(list_t* mapping, char* FMCtoKey , char* phrase, FILE *outstream) {
    //call createSecretKey with FMCtoKey and phrase to build secret key

}

