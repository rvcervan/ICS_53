#include <unistd.h>
#include <stdio.h>
void main(){
    int i = 0;
    while(i < 20){
        //printf("%d\n", i);
        ++i;
        sleep(1);
    }
    return;
}
