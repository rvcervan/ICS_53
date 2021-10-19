#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main() {
  if (fork() == 0) {
    while(1)
    /*this is the child*/
        printf("Terminating child : PID=%d\n", getpid());
        sleep(10);
  }

  /* parent code */
  
  printf("Running parent : PID=%d\n", getpid());
  exit(1);
  
}
