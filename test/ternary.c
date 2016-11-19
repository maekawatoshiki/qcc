#include <stdio.h>

int test() {
  int i = 1;
  if((i ? 2 : 3) != 2) return 1;
  return 0;
}

