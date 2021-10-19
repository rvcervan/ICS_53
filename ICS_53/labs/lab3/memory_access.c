#include <stdio.h>
#include <stdlib.h>
#include <string.h>


struct l {
  unsigned long data0;
};

struct p {
  void *data0;
};

struct i {
  unsigned int data0;
};

struct s {
  unsigned short data0;
  unsigned short data1;
  unsigned short data2;
};

struct c {
  unsigned char data0;
  unsigned char data1;
  unsigned char data2;
  unsigned char data3;
  unsigned char data4;
};

struct b {
  unsigned int data0 : 1;
  unsigned int data1 : 2;
  unsigned int data2 : 4;
  unsigned int data3 : 8;
};

int main(int agrc, char **argv) {
  unsigned long var = 0xBEEFCAFE;
  void *vp = &var;
  printf("Value of var: 0x%lx\n", var);
  printf("Address of var: %p\n\n", vp);

  printf("Value of data1 field when the address of var (%p) is " 
         "visualized as struct b: %0x\n",
         vp, ((struct b *)vp)->data1);

  return 0;
}
