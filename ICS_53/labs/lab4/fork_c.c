#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main() {
  pid_t pid;
  int x = 10;

  pid = fork();
  if (pid == 0) {
    /*this is the child*/
    sleep(1);
    printf("child : x=%d\n", ++x);
    exit(0);
  }

  /* parent code */
  
  printf("parent : x=%d\n", --x);

  exit(0);
}
