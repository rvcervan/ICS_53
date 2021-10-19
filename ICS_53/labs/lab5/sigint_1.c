#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>

int cnt = 0;

void unix_error(char *msg)
{
    fprintf(stderr, "%s: %s\n", msg, strerror(errno));
    exit(0);
}

void sigint_handler(int sig)
{
    ++cnt;
    printf("You pushed Ctrl-C! %d\n", cnt);
    if(cnt == 10){
	       exit(0);
    }else{
	       sleep(3);
    }
}

int main()
{
    if (signal(SIGINT, sigint_handler) == SIG_ERR)
	unix_error("signal error");

    while(1)
	   pause();

    return 0;
}
