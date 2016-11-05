#include <stdio.h>

int test();

int main() {
  int ret_code = test();
  printf(TEST_NAME " ... \x1b[32mOK\x1b[39m\n");
  return ret_code;
}
