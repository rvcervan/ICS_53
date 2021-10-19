#include<stdio.h>
#include<stdlib.h>

int main (void) 
{
    // Reference: http://www.asciitable.com/
    char abc[12] = {'a','b','c', 100, 101, 102, 103, 104, 0x69, 0x6A, 0x6B, 0x6C};
    char abc2[] = "aa";
    char abc3[5] = "B";
    printf("%s\n", abc);
    printf("Answer Question 10 & 11 & 12\n");

    char *p = &abc[3];
    printf("Answer Question 13\n");

    char* strings[4] = {"abcdefg", "123", "!@#", "000"};
    printf("Size of char * in bytes: %ld\n", sizeof(char*));
    printf("Answer Question 14 & 15\n");

    int nums[9] = {97, 98, 99, 100, 101, 102, 103, 104, 0};
    printf("Answer Question 16 & 17\n");

    return 0;
}
