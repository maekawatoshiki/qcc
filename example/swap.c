#include <stdio.h>

void swap(int *a, int *b) {
  int t;
  t = *b;
  *b = *a;
  *a = t;
  return;
}

int main() {
  int a, b;
  a = 10;
  b = 20;
  swap(&a, &b);
  printf("%d, %d%c", a, b, 10);
  return 0;
}
