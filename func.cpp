#include "func.hpp"

void FunctionList::add(func_t f) {
  func_list.push_back(f);
}

func_t *FunctionList::get(std::string name) {
  for(auto &func : func_list) {
    if(func.name == name) return &func;
  }
  return nullptr;
}
