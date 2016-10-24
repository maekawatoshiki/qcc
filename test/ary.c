#include <stdio.h>

int test() {
  int a[10];
  a[1] = 65;
  char s[10];
  s[0] = a[1];
  return 0;
}

