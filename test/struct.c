#include <stdio.h>

struct vec {
  int x, y;
};

void func(struct vec *v) {
  v->x = 10;
  v->y += v->x;
}

int test() {
  struct vec v;
  v.y = 10;
  func(&v);
  return 0;
}
