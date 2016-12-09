#include <stdio.h>
#include <stdlib.h>

void mat_mul(int F[2][2], int M[2][2]);
void mat_pow(int F[2][2], int n);

int fibo(int n) {
  int F[2][2] = {{1,1},{1,0}};
  if(n == 0) return 0;
  mat_pow(F, n-1);
  return F[0][0];
}

void mat_mul(int F[2][2], int M[2][2]) {
  int x =  F[0][0]*M[0][0] + F[0][1]*M[1][0];
  int y =  F[0][0]*M[0][1] + F[0][1]*M[1][1];
  int z =  F[1][0]*M[0][0] + F[1][1]*M[1][0];
  int w =  F[1][0]*M[0][1] + F[1][1]*M[1][1];

  F[0][0] = x;
  F[0][1] = y;
  F[1][0] = z;
  F[1][1] = w;
}

void mat_pow(int F[2][2], int n) {
  int M[2][2] = {{1,1},{1,0}};
  for(int i = 2; i <= n; i++)
    mat_mul(F, M);
}

int main(int argc, char *argv[]) {
  if(argc < 2) {
    puts("usage: ./fibo [NUMBER]");
    return 0;
  }

  for(int i = 1; i <= atoi(argv[1]); i++)
    printf("%d%c", fibo(i), 10);
  return 0;
}
