#include "var.hpp"

void VariableList::add(var_t v) {
  var_list.push_back(v);
}

var_t *VariableList::get(std::string name) {
  for(auto &v : var_list) 
    if(v.name == name) return &v;
  return nullptr;
}
