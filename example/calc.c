/*
 * Calculator 
 * - this code is written in C that qcc supports
 * - usage: 
 *     input: an expression WITHOUT SPACES. e.g. 1+2*3 (not 1 + 2 * 3)
 *            brackets can be used. e.g. 2*(1/2)
 *            double   can be used. e.g. 1.3/43
 *     output: show nodes e.g. (+ 1 (* 2 3) ) and the answer
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

enum NODE_KIND {
  KIND_NUM,
  KIND_OP
};

char *str; // input 

typedef struct node_t {
  enum NODE_KIND kind;

  double num;
  struct {
    char op;
    struct node_t *left, *right;
  } op;
} node_t;

node_t *make_number(double n) {
  node_t *num = malloc(sizeof(node_t));
  num->kind = KIND_NUM;
  num->num = n;
  return num;
}
node_t *make_op(char op, node_t *left, node_t *right) {
  node_t *num = malloc(sizeof(node_t));
  num->kind = KIND_OP;
  num->op.op = op;
  num->op.left = left;
  num->op.right = right;
  return num;
}

node_t *expr_addsub();

node_t *expr_number() {
  if(*str == '(') {
    str++;
    node_t *n = expr_addsub();
    str++;
    return n;
  }
  if(0 == isdigit(*str)) return NULL; // unary operator '!' is still unsupported
  char buf[16], *ptr = buf;
  for(int i = 0; i < 16; i++) buf[i] = 0;
  while(isdigit(*str) || *str=='.') *ptr++ = *str++;
  return make_number(atof(buf));
}

node_t *expr_muldiv() {
  node_t *left = expr_number();
  while(*str == '*' || *str == '/') {
    char op = *str++;
    node_t *right = expr_number();
    left = make_op(op, left, right); 
  }
  return left;
}

node_t *expr_addsub() {
  node_t *left = expr_muldiv();
  while(*str == '+' || *str == '-') {
    char op = *str++;
    node_t *right = expr_muldiv();
    left = make_op(op, left, right); 
  }
  return left;
}

double calc(node_t *node) {
  if(node->kind == KIND_OP) {
    double l = calc(node->op.left), r = calc(node->op.right);
    if(node->op.op == '+') return l + r;
    if(node->op.op == '-') return l - r;
    if(node->op.op == '*') return l * r;
    if(node->op.op == '/') return l / r;
  } else return node->num;
}

void show(node_t *node) {
  if(node->kind == KIND_OP) {
    printf("(%c ", node->op.op);
    show(node->op.left);
    show(node->op.right);
    printf(") ");
  } else printf("%.10g ", node->num);
}

int main() {
  char buf[100];
  puts("input an expression without spaces - e.g. 3/2+12*(2-3)");
  scanf("%99s", buf);
  str = buf;

  node_t *node = expr_addsub();
  show(node); puts("");

  printf("%.10g%c", calc(node), 10);
  return 0;
}
