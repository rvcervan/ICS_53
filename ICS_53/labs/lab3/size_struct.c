#include <stdio.h>
#include <stdlib.h>

struct foo {
  char c[5];
};

struct foo1 {
  char c[5];
  short data;
};

struct foo2 {
  char c[5];
  int data;
};

struct foo3 {
  char c[5];
  long data;
};

struct foo4 {
  char c[5];
  long data;
  char x;
};

int main(int argc, char **argv) {
  printf("sizeof(struct foo)  = %zu\n", sizeof(struct foo));
  printf("sizeof(struct foo1) = %zu\n", sizeof(struct foo1));
  printf("sizeof(struct foo2) = %zu\n", sizeof(struct foo2));
  printf("sizeof(struct foo3) = %zu\n", sizeof(struct foo3));
  printf("sizeof(struct foo4) = %zu\n", sizeof(struct foo4));
}
