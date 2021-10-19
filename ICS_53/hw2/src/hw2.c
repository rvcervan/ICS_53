// Raul Cervantes
// rvcervan

#include "hw2.h"

// Part 1  Functions to implement
void createSecretKey(char* keyphrase, char* key) {

    if(!keyphrase || !key){ return; }

    //check if keyphrase is empty
    //if keyphrase is empty, key is reverse alphabet in Capital.
    char *alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    char *reverse = "ZYXWVUTSRQPONMLKJIHGFEDCBA";

    
    if (*keyphrase == '\0'){ //keyphrase pointer is "", is empty
        
        int i;
        for(i = 0; i < 26; ++i){
            *(key + i) = *(reverse+i);
        }

    }
    //if keyphrase is not empty, iterate through keyphrase and append each letter to in CAPS in the order they appear. no dupes
    //then iterate through alphabet and append the non-dupes to key in order they appear.
    else{
        
        char *c = (char *) malloc(2*sizeof(char));
        
        
        *c = *keyphrase;
        int counter = 0;

        int arrayIndex = 0;
        
        int inside;

        while(*c != '\0'){
            inside = 0;
            //*c = *(keyphrase+counter);
            if(*c > 96 && *c < 123){
                *c = *c-32;
            }
            //printf("%c", *c);

            if(*c > 64 && *c < 91){
                //first check for dupes in key
                int i;
                for(i = 0; i < arrayIndex; ++i){
                    //printf("%c", *(key+i));
                    if(*c == *(key+i)){
                        
                        inside = 1;
                    }
                }
                if(inside == 0){
                    *(key + arrayIndex) = *c;
                    arrayIndex++;
                }
               

            }
            counter++;
            *c = *(keyphrase+counter);

        }

        *c = *alphabet;

        counter = 0;

        while(*c != '\0'){
            inside = 0;
            //*c = *(keyphrase+counter);
            if(*c > 96 && *c < 123){
                *c = *c-32;
            }

            if(*c > 64 && *c < 91){
                //first check for dupes in key
                int i;
                for(i = 0; i < arrayIndex; ++i){
                    //printf("%c", *(key+i));
                    if(*c == *(key+i)){
                        
                        inside = 1;
                    }
                }
                if(inside == 0){
                    *(key + arrayIndex) = *c;
                    arrayIndex++;
                }
               

            }
            counter++;
            *c = *(alphabet+counter);

        }

        *(key + arrayIndex) = '\0'; 
        

        free(c);
        
    }
    
}
    

char morseToKey(char* mcmsg, char* FMCtoKey, char* key) {

    if(!mcmsg || !FMCtoKey || !key){ return -1; }
    
    int counter = 0;
    int found = 0;

    while(*(FMCtoKey + counter) != '\0'){
        
        if(*(mcmsg+0) == *(FMCtoKey+counter)){

            if(*(FMCtoKey+(counter+1)) == '\0'){ break; }
            
            if(*(mcmsg+1) == *(FMCtoKey+(counter+1))){
                
                if(*(FMCtoKey+(counter+2)) == '\0') { break; }

                if(*(mcmsg+2) == *(FMCtoKey+(counter+2))){
                    found = 1;
                    break;
                }
            }
        }

        counter = counter + 3;
    }


    if(found == 0){ return -1; }

    int div = 3;
    counter = counter/div;
    
    return *(key+counter);
}


// Part 2 Functions to implement

void mapping_tPrinter(void* val_ref) {

    if( !val_ref ) { return; }    

    fprintf(stderr, "%c: %s", ((mapping_t*)val_ref)->ASCII, ((mapping_t*)val_ref)->sequence);

}

int mapping_tComparator(void* lhs, void* rhs) {

    if( !lhs || !rhs ){ return 0; }

    if( ((mapping_t*)lhs)->ASCII < ((mapping_t*)rhs)->ASCII ){ return -1; }
    if( ((mapping_t*)lhs)->ASCII == ((mapping_t*)rhs)->ASCII ){ return 0; }
    if( ((mapping_t*)lhs)->ASCII > ((mapping_t*)rhs)->ASCII ){ return 1; }

}

void mapping_tDeleter(void* val_ref) {

    if( !val_ref ){ return; }

    
    free(((mapping_t*)val_ref)->sequence);
    free(val_ref);

}

void DestroyList(list_t* list) {
    if( !list ){ return; }
    
    node_t *head = list->head;
    
    while(head != NULL){
        node_t *temp = head->next;
        list->deleter(head->data);
        free(head);
        head = temp;
    }


    free(list);

}

mapping_t* CreateMapping(char* line) {

    if( *line == '\0' ) {return NULL;}

    mapping_t *map = malloc(sizeof(mapping_t));
    
    char *s = (char*) malloc(8*sizeof(char));

    int counter = 0;
    int i = 0;

    int space = 0;

    //printf("%s\n", line);
    while(*(line + counter) != '\n'){

        if(*(line+counter) == '\0'){break;}
        
        if(((*(line+counter) > 64  && *(line+counter) < 91) || (*(line+counter) > 47 && *(line+counter) < 58) ||
            *(line+counter) == 46 || *(line+counter) == 44 || *(line+counter) == 58 || *(line+counter) == 34 ||
            *(line+counter) == 39 || *(line+counter) == 33 || *(line+counter) == 63 || *(line+counter) == 64 ||
            *(line+counter) == 45 || *(line+counter) == 59 || *(line+counter) == 40 || *(line+counter) == 41 ||
            *(line+counter) == 61) && space == 0){

            map->ASCII = *(line + counter);
            
        }

        if(*(line+counter) == 32){ space = 1; }

        if((*(line+counter) == 46 || *(line+counter) == 45) && space == 1){
            *(s + i) = *(line+counter);
            i++;
        }
        counter++;
    }
    //printf("heere");
    *(s+i) = '\0';
    //*(s+i) = *(line+counter);
    map->sequence = s;
    
    

    return map;

}

list_t* GenerateFMCMappings(char* filename) {
    
    //Check for duplicates somehow.

    char* used = (char*) malloc(60*sizeof(char));
    int counter = 0;
    FILE *disc;
    disc = fopen(filename, "r");
    if(disc == NULL){
        printf("Error in opening!\n");
        return NULL;
    }

    char *buffer;
    size_t bufsize = 0;
    ssize_t line_size;

    line_size = getline(&buffer, &bufsize, disc);

    *(used + counter) = *buffer;
    counter++;
    
    list_t* myList = CreateList(&mapping_tComparator, &mapping_tPrinter, &mapping_tDeleter);
    /*
    mapping_t* mapping1 = CreateMapping("A .-\n");
    InsertInOrder(myList, (void*) mapping1);
    */
    
    while(line_size >= 0){

        mapping_t* mapping = CreateMapping(buffer);

        InsertInOrder(myList, (void*) mapping);

        line_size = getline(&buffer, &bufsize, disc);

        int i;
        for(i = 0; i < counter; ++i){
            if(*(used+i) == *buffer){
                line_size = getline(&buffer, &bufsize, disc);
                continue;
            }
        }
    }
    free(used);
    
    fclose(disc);

    return myList;



}

mapping_t* FindASCII(char token, list_t* list) {

    if(!list){ return NULL; }

    mapping_t *map = NULL;
    node_t *head = list->head;

    while(head != NULL){
        node_t *temp = head->next;
        map = head->data;
        if(token == map->ASCII){
            return map;
        }
        

        head = temp;
    }
    return NULL;

}

int createMorse(list_t* mapping, char **mc_ptr) {
    
    //plain text message is in STDIN
    //'X' means stop and look for the next sequence
    //'XX' means space, and a stop as well. 
    //consecutive spaces are still treated as a singe XX
    //end of message must have XX
    //any spaces between end of message and EOF are ignored.
    //lowercase chars are converted to upper
    //characters that do not have mappings are ignored.
    if(mapping == NULL){ *mc_ptr = NULL; return -1; }
    

    int c = getchar();
    mapping_t *map = NULL;

    size_t size = 8;
    //char *morse = (char*) malloc(size * sizeof(char));

    //if space = 1, then previous char was a space and we ignore inserting XX
    int space = 0;
    
    int saved_position = 0;
    *mc_ptr = (char*) calloc(0, sizeof(char));
    *mc_ptr = (char*) realloc(*mc_ptr, size);
    if(*mc_ptr == NULL){
        return -1;
    }

    
     
    while(c != EOF){
        //if c is a letter and lowercase, change to uppercase

        //if c is a space XX is placed and use a check to see if any more spaces are used
        //since multiple spaces in a row is still XX

        if(c == 32 && space == 0){
            *((*mc_ptr)+saved_position) = 'x';
            ++saved_position;
           

            space = 1;
        }

        if(c > 96 && c < 123){
            c = c - 32;
        }
        
        map = FindASCII(c, mapping);
        int i = 0;
        
        if(map == NULL){
            c = getchar();
            continue;
        }
        
        else if(map != NULL){
            //sequence is found

            
            while(*((map->sequence)+i) != '\0'){
                *((*mc_ptr) + saved_position) = *((map->sequence)+i);
                ++i;
                ++saved_position;
            }
            

            //after sequence, place X.
            *((*mc_ptr)+saved_position) = 'x';
            ++saved_position;

            //realloc()
            size = size + 8;
            *mc_ptr = (char*) realloc(*mc_ptr, size);

            c = getchar();
            space = 0;

            
        }
        
        //c = getchar();

    }
    *((*mc_ptr)+saved_position) = 'x';
    ++saved_position;
    
    *((*mc_ptr)+saved_position) = '\0';
    
    return 0;
    
}

// Part 3 Functions to implement
int encrypt(list_t* mapping, char* FMCtoKey , char* phrase, FILE *outstream) {
    //call createMorse to get morseMessage and then create 

    //char morseToKey(char* morseMessage, char* FMCtoKey, char* keyArray)

    char *keyArray = (char*) malloc(27 * sizeof(char));

    createSecretKey(phrase, keyArray);

    char *mc_ptr;
    
    int status = 0;
    status = createMorse(mapping, &mc_ptr);
    if(status == -1){
        return status;
    }
    int position=0;
    char c = 0;
    while(c != -1){
        c = morseToKey(mc_ptr+position, FMCtoKey, keyArray);
        if(c == -1){ break;}
        
        fprintf(outstream, "%c", c);
        position = position+3;
    }
    free(mc_ptr);
    free(keyArray);
    //fprintf(stdout, "%c", '+');

    return 0;

}
