#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main() {
  pid_t pid;

  printf("About to call fork() one time\n");

  pid = fork();

  if (pid == 0) {
    printf("child: After the call to fork()\n");
  } else {
    sleep(1);
    printf("parent: After the call to fork()\n");
  }

  exit(0);
}
