#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main(int agrc, char **argv) {
    int i, arrayA[3] = {0,1,2};
    int *stuff = malloc(3*sizeof(int));    //declare space to hold 3 integers
    printf("address of arrayA: %p\taddress of stuff: %p\n", &arrayA[0], &stuff[0]);
    
    for(i = 0; i < 3; i++)
        printf("stuff[%d]: %d\n", i, stuff[i]);
    printf("\n");

    free(stuff); //Uncomment for Question 26 

    stuff[0] = 0xFFFFFFFF;
    stuff[1] = 0xCAFEBABE;
    stuff[2] = 0XDEADBEEF;

    for(i = 0; i < 3; i++)
        printf("stuff[%d]: %d\n", i, stuff[i]);
    free(stuff); //Comment out this line for Question 26. Then uncomment it for Question 27

    int *arrayB = malloc(5*sizeof(int));
    printf("\naddress of arrayB: %p\n", &arrayB[0]);
    *arrayB = 9;
    *(arrayB + 1) = 8;
    
    for(i = 0; i < 5; i++)
        printf("arrayB[%d]: %d\n", i, arrayB[i]);

    free(arrayB);
    return 0;
}
