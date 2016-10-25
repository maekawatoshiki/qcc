#include <stdio.h>

int test();

int main() {
  test();
  printf(TEST_NAME " ... \x1b[32mOK\x1b[39m\n");
  return 0;
}
