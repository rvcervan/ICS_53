#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 10
typedef int array_t[N][N];

int fillArray(array_t a)
{
        int i, j;

        for (i = 0; i < N; i++) {
            for (j = 0;  j < N; j++) {
                a[i][j] = rand();

                printf("%x, ", (((int) &a[i][j]) & 0x00000FFF));   // Store value of a[i][j]
            }
            printf("\n");
        }
        printf("\n");
}

int sumA(array_t a)
{
        int i, j;
        int sum = 0;

        for (j = 0; j < N; j++) {
            for (i = 0;  i < N; i++) {
                sum += a[i][j]; 

                printf("%x, ", (((int) &a[i][j]) & 0x00000FFF));   // load value of a[i][j]
            }
            printf("\n");
        }
        printf("\n");
        return sum;
}

int sumB(array_t a)
{
        int i, j;
        int sum = 0;

        for (j = 0; j < N; j+=2) {
            for (i = 0;  i < N; i+=2) {
                sum += (a[i][j] + a[i+1][j] + a[i][j+1] + a[i+1][j+1]);

                printf("%x, ", (((int) &a[i][j]) & 0x00000FFF));  // load value of a[i][j]
                printf("%x, ", (((int) &a[i+1][j]) & 0x00000FFF));   // load value of a[i+1][j]
                printf("%x, ", (((int) &a[i][j+1]) & 0x00000FFF)); // load value of a[i][j+1]
                printf("%x, ", (((int) &a[i+1][j+1]) & 0x00000FFF));  // load value of a[i+1][j+1]
                printf("\n");
            }
        }
        printf("\n");
        return sum;
}

int main(int argc, char* argv) {

    array_t testarray; 

    srand(time(0)); 
    fillArray(testarray);
    printf("Array filled.\n\n");
    int sum = sumA(testarray);
    printf("The sum using sumA is: %d\n\n", sum);

    sum = sumB(testarray);
    printf("The sum using sumB is: %d\n\n", sum);

    return 0;

}
