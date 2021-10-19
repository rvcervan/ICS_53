//debug2.c

#include <stdio.h>
#include <stdlib.h>

int factorial(int num);

int main(int argc, char *argv[]){
    int value, result;

    printf("Please enter a positive number to compute the factorial of: ");
    scanf("%d", &value);
    // You should error check this...
    result = factorial(value);
    printf("The result of %d! is %d\n", value , result);
    return EXIT_SUCCESS;
}

int factorial(int num){
    if (num <= 1){
        return 1;
    } else {
        return num * factorial(num-1);
    }
}
