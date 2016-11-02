#include <stdio.h>

struct vec {
  int x, y;
};

typedef struct {
  int x, y;
} pair;

void func(struct vec *v) {
  v->x = 10;
  v->y += v->x;
}

int add_pair(pair *p) {
  return p->x + p->y;
}

int test() {
  struct vec v;
  pair p;
  p.x = p.y = 23;
  v.y = 10;
  func(&v);
  add_pair(&p);
  return 0;
}
