#include <stdio.h>

int test() {
  int a, b, c, i = 10;
  a = b = c = 100 + i;
  {
    int q = 10;
  }
  int a[] = {1, 2};
  return 0;
}
