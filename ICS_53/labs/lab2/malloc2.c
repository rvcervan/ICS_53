#include <stdio.h>
#include <stdlib.h>

int main() {
    int* a = malloc(150);
    int* b = malloc(10);
    int* c = malloc(7);
    free(b);
    int* d = malloc(10);

    printf("a:%p\nb:%p\nc:%p\nd:%p\n",a,b,c,d);

    return 0;
}
