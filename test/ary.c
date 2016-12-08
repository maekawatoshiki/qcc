#include <stdio.h>

int test() {
  int a[10];
  a[1] = 65;
  char s[10];
  s[0] = a[1];
  int a[4] = {1, 3, 5, 12};
  if(a[0] != 1)  return 1;
  if(a[1] != 3)  return 1;
  if(a[2] != 5)  return 1;
  if(a[3] != 12) return 1;
  return 0;
}

