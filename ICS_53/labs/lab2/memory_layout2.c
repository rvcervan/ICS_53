#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void foo(){
    int a;
    int b;
    int c;

    a = 98;
    b = 1024;
    c = a * b;
    printf("[in foo] Value in a: %d\tb:%d\tc:%d\n", a, b, c);
    printf("[in foo] Address of a: %p\tb: %p\tc: %p\n", (void *)&a, (void *)&b, (void *)&c);
}

int main(int argc, char **argv, char **env) {
    int A;
    void *B;
    int i;
    char* ch = "Hi!!";
    int * m = malloc(4);
    *m = 10;

    printf("[in main] Address of A: %p\tB: %p\ti: %p\tch: %p\tm: %p\n", &A, B, &i, &ch, m);
    printf("[in main] Address of string \"Hi!!\":%p\n", ch);

    printf("Address of the array of arguments: %p\n", (void *)argv);
    printf("Addresses of the arguments:\n\t");
    
    for (i = 0; i < argc; i++)
        printf("[%s]:%p ", argv[i], argv[i]);
    printf("\n");

    foo();
    getchar();    

  return 0;
}
