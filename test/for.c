#include <stdio.h>

int test() {
  int i, sum;
  sum = 0;
  for(i = 1; i <= 10; i += 1) 
    sum += i;
  return sum;
}
