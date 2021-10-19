#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>

void unix_error(char *msg)
{
    fprintf(stderr, "%s: %s\n", msg, strerror(errno));
    exit(0);
}

void sigint_handler(int sig)
{
    printf("You pushed Ctrl-C!\n");
    exit(0);
}

int main()
{
    sigset_t mask, prev_mask;

    sigemptyset(&mask);   //WHy do we empty the set???
    sigaddset(&mask, SIGINT);

    sigprocmask(SIG_BLOCK, &mask, &prev_mask);

    if (signal(SIGINT, sigint_handler) == SIG_ERR)
	   unix_error("signal error");

    while(1)
	   pause();

    return 0;
}
