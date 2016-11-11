#include <stdio.h> 
#define N 8
#define true 1
#define false 0

void print (int mat[][N], int q[]) {
  int i = 0;
  int j = 0;
  int p = 0;
  puts("BOARD:");
  for (i = 0; i < N; i++) {
    p = q[i];
    for (j = 0; j < N; j++)
    {
      if (p == j)
        mat[j][i] = 1;
      if(mat[j][i]) printf ("Q");
      else printf(".");
    }
    puts("");
  }
  return;
}
int isAttacked (int q[], int n) {
  int i = 0;
  for (i = 0; i < n; i++)
  {
    if (q[i] == q[n])
      return false; // same column
    if ((q[i] - q[n]) == (n - i))
      return false; // same major diagonal
    if ((q[n] - q[i]) == (n - i))
      return false; // same minor diagonal
  }
  return true;
}
void zeroIt (int chess[][N]) {
  int i, j;
  for (i = 0; i < N; i++)
    for (j = 0; j < N; j++)
      chess[i][j] = 0;
  return;
}
void solve (int q[], int col, int chess[][N]) {
  if (col == N)
  {
    print(chess, q);
    zeroIt (chess);
  }
  else
  {
    int i = 0;
    for (i = 0; i < N; i++)
    {
      q[col] = i;
      if (isAttacked (q, col))
      {
        solve (q, col + 1, chess);
      }
    }
  }
  return;
}

int main () { 
  int chess[N][N];
  zeroIt (chess);
  int a[N];
  solve (a, 0, chess);
  return 0;
}

