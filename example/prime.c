#include <stdio.h>

#define NUM 1000

void prime(int table[]) {
  int i, j;
  for(i = 0; i < NUM; i++)
    table[i] = 1;  
  table[0] = 0; 

  for(i = 2; i * i < NUM; i++) {
    if(table[i] == 1) {
      for(j = i * 2; j < NUM; j += i){
        table[j] = 0;
      }
    }
  }
}

void show(int table[]) {
  int count = 1;
  for(int i = 2; i < NUM; i++) {
    if(table[i] == 1) {
      printf("%4d", i);
      count++;
    }
    if(count % 10 == 0) { puts(""); count++; }
  }
  puts("");
}

int main() {
  int table[NUM];

  prime(table);
  show(table);
  
  return 0;
}
