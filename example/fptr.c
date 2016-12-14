#include <stdio.h>

int add(int a, int b) { return a + b; }
int sub(int a, int b) { return a - b; }

int (*func_table[])(int, int) = {
  add,
  sub
};

int main() {
  puts("* function pointer sample - you know usage if you read code *");
  int cur_val = 0;
  while(1) {
    int input;
    printf("current value: %d%c", cur_val, 10);
    printf("exit: 0, add: 1, sub: 2 -- ");
    scanf("%d", &input);
    if(input == 0) return 0;
    
    cur_val = func_table[input-1](cur_val, 1);
  }
}
