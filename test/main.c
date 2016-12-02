#include <stdio.h>

int test();

int main() {
  int ret_code = test();
  if(ret_code == 0)
    printf(TEST_NAME " ... \x1b[32mOK\x1b[39m\n");
  else
    printf(TEST_NAME " ... \x1b[31mFAILED\x1b[39m\n");
  return ret_code;
}
