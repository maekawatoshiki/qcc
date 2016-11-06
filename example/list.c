#include <stdio.h>

void *malloc(int);
void free(void *);

typedef struct list_t {
  struct list_t *next;
  int val;
} list;

int count(list *lst) {
  int count = 0;
  while(lst) {
    lst = lst->next;
    count++;
  }
  return count;
}

void append(list *lst, int val) {
  if(lst->next) {
    append(lst->next, val);
  } else {
    lst->next = malloc(8);
    lst->next->val = val;
  }
  return;
}

void show(list *lst) {
  printf("(%d) ", lst->val);
  if(lst->next) show(lst->next);
  return;
}

int main() {
  list lst;
  lst.next = NULL;
  lst.val = 12;
  append(&lst, 2);
  append(&lst, 5);
  printf("%d%c", count(&lst), 10);
  show(&lst);
  return 0;
}
