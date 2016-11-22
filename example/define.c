#include <stdio.h>
#define OnePlusTwo (1 + 2)
#define Three 3

#define NUM 10
int f() {
  return NUM;
}
#undef

int main() {
  if(OnePlusTwo == Three) puts("True");
  return 0;
}
