#include <stdio.h>
#include <stdlib.h>

#define MAX 4


typedef struct {
  int width, height;
} rect_t;

int cmp_rect(void *a, void *b) {
  return ((rect_t *)a)->width * ((rect_t *)a)->height -
          ((rect_t *)b)->width * ((rect_t *)b)->height;
}

void show(rect_t rect[]) {
  for(int i = 0; i < MAX; i++) 
    printf("(%d, %d) [%d]%c", rect[i].width, rect[i].height, rect[i].width * rect[i].height, 10);
}

int main() {
  rect_t rect[] = {
    {5, 7},
    {2, 2},
    {3, 4},
    {1, 2}
  };
  puts("before:");
  show(rect);
  qsort(rect, MAX, sizeof(rect_t), cmp_rect);
  puts("after:");
  show(rect);
}
