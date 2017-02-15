#include "func.hpp"

block_t *BlockList::create_new_block() {
  block_t *newb = new block_t;
  newb->parent = current;
  if(current) current->children.push_back(newb);
  current = newb;
  return newb;
}

block_t *BlockList::escape_block() {
  if(current->parent) {
    current = current->parent;
    return current;
  }
  return nullptr;
}

block_t *BlockList::get_current() {
  return current;
}

VariableList *BlockList::get_varlist() {
  return &current->var_list;
}

var_t *BlockList::lookup_var(std::string name) {
  block_t *cur = current;
  var_t *v = cur->var_list.get(name);
  while(!v) {
    cur = cur->parent;
    if(!cur) break;
    v = cur->var_list.get(name);
  }
  return v;
}

void FunctionList::add(func_t f) {
  func_map[f.name] = (f);
}

func_t *FunctionList::get(std::string name) {
  return func_map.count(name) ? &(func_map[name]) : nullptr;
}
