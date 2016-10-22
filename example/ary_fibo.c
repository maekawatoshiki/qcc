#include <stdio.h>

int main() {
  int a[10], i;
  a[0] = 1;
  a[1] = 1;
  i = 2;
  while(i < 10) {
    a[i] = a[i - 1] + a[i - 2];
    i += 1;
  }
  i = 0;
  while(i < 10) {
    printf("%d ", a[i]);
    i += 1;
  }
  return 0;
}
