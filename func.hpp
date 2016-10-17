#pragma once

#include "common.hpp"
#include "type.hpp"
#include "var.hpp"

struct func_t {
  std::string name;
  Type *ret_type;
  std::vector<Type *> args_type;
  std::vector<llvm::Type *> llvm_args_type;
  std::vector<std::string> args_name;
  llvm::Function *llvm_function;
  VariableList var_list;
};

class FunctionList {
  private:
    std::vector<func_t> func_list;
  public:
    void add(func_t);
    func_t *get(std::string);
};
