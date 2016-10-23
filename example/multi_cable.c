#include <stdio.h>

void make_9x9(int a[][9]) {
  int i, k;
  i = 0;
  while(i < 9) {
    k = 0;
    while(k < 9) {
      a[i][k] = (i + 1) * (k + 1);
      k += 1;
    }
    i += 1;
  }
  return;
}

int main() {
  int a[9][9], i, k;
  make_9x9(a);
  i = 0;
  while(i < 9) {
    k = 0;
    while(k < 9) {
      printf("%d ", a[i][k]);
      k += 1;
    }
    puts("");
    i += 1;
  }

  return 0;
}
