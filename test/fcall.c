#include <stdio.h>

int f() {
  return 123;
}

int test() {
  int ret;
  ret = f();
  return 0;
}
