#pragma once

#include "common.hpp"
#include "type.hpp"

struct var_t {
  var_t(std::string _name, Type *_type):
    name(_name), type(_type) {};
  Type *type;
  llvm::Value *val;
  std::string name;
};

class VariableList {
  private:
    std::vector<var_t> var_list;
  public:
    void add(var_t);
    var_t *get(std::string name);
};
