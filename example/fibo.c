#include <stdio.h>

int fibo(int n) {
  if(n < 2) 
    return 1;
  else
    return fibo(n - 2) + fibo(n - 1);
}

int main() {
  int n;
  n = 45;
  printf("fibo(%d) = %d%c", n, fibo(n), 10);
  return 0;
}
