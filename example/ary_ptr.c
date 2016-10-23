#include <stdio.h>

int main() {
  char a[3], *b;
  a[0] = 65;
  a[1] = 10;
  a[2] = 0;
  b = "hello";
  printf("%s %s", b, a);

  return 0;
}
