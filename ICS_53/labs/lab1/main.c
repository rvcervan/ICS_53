#include <stdio.h>
#include <string.h>

int main(){

    char * netid = "rvcervan";
    int class = 53;
    printf("Hello, %s! Welcome to ICS%d!\n", netid, class);
    
    for(int i = strlen(netid); i >= 0; --i){
        printf("%d\n", i);
    }
    return 0;
}
