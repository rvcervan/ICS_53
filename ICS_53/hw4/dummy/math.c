#include <stdio.h>
#include <string.h>
#include <math.h>

int main(int argc, char* argv[]){
    

    size_t i = 8192;
    while(1){
       
        printf("%d is i\n", (int)i);

        double x = (int)i/16.00;
        
        int trunc = (int)x;
        if(trunc == x){
            printf("%d\n", (int)i);
            return i;
        }
        ++i;

    }



    return -1;
}
