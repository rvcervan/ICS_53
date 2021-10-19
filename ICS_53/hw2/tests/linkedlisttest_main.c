#include <stdio.h> 
#include <stdlib.h> 
#include "hw2.h"
#include "linkedlist.h"

int main() 
{ 
    list_t* myList = CreateList(&mapping_tComparator, &mapping_tPrinter, &mapping_tDeleter);
    
    mapping_t* mapping1 = CreateMapping(". .....\n");
    InsertInOrder(myList, (void*) mapping1);

    printf("******Character .******\n");    
    PrintLinkedList(myList);

    char temp[] = ". .....\n";
    mapping_t* mapping2 = CreateMapping(temp);
    InsertInOrder(myList, (void*) mapping2);

    printf("******A -> C******\n");    
    PrintLinkedList(myList);

    char* temp3 = "0 -----\n";
    mapping_t* mapping3 = CreateMapping(temp3);
    InsertInOrder(myList, (void*) mapping3);
    char temp4[15] = "S ...\n";
    mapping_t* mapping4 = CreateMapping(temp4);
    InsertInOrder(myList, (void*) mapping4);

    printf("******0 -> A -> C -> S ******\n");  
    PrintLinkedList(myList);

    DestroyList(myList);
    myList = NULL;

    printf("******After Destroy List******\n");
    if(myList == NULL)
        printf("myList is deallocated! But how?!?\n"); 
    else
        printf("myList still exists..... or does it?!?!\n");
    // Valgrind to the rescue!

    //This should print nothing!
    PrintLinkedList(myList);

    printf("******************************\n");

    return 0; 
} 
