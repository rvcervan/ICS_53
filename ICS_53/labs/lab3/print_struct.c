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

void print_struct(void *opaque, size_t size) {
  printf("%-14s: Byte\n", "Address");
  int i;
  for ( i = 0; i < size; i++) {
    unsigned char *c = opaque;
    printf("%p: %02x ", c + i, c[i]);
    if (c[i] != 0xcc && c[i] != 0xdd)
      printf("(padding)");
    printf("\n");
  }
  printf("\n");
}

int main(int argc, char **argv) {
  struct foo f;
  int i;
  for (i = 0; i < 5; i++)
    f.c[i] = 0xcc;
  printf("struct foo:\n");
  print_struct(&f, sizeof(f));

  struct foo1 f1;
  for (i = 0; i < 5; i++)
    f1.c[i] = 0xcc;
  f1.data = 0xdddd;
  printf("struct foo1:\n");
  print_struct(&f1, sizeof(f1));

  struct foo2 f2;
  for (i = 0; i < 5; i++)
    f2.c[i] = 0xcc;
  f2.data = 0xdddddddd;
  printf("struct foo2:\n");
  print_struct(&f2, sizeof(f2));

  struct foo3 f3;
  for (i = 0; i < 5; i++)
    f3.c[i] = 0xcc;
  f3.data = 0xdddddddddddddddd;
  printf("struct foo3:\n");
  print_struct(&f3, sizeof(f3));

  struct foo4 f4;
  for (i = 0; i < 5; i++)
    f4.c[i] = 0xcc;
  f4.data = 0xdddddddddddddddd;
  f4.x = 0xcc;
  printf("struct foo4:\n");
  print_struct(&f4, sizeof(f4));
  return 0;
}
