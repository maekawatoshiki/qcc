#pragma once

#include "common.hpp"
#include "type.hpp"
#include "var.hpp"

struct block_t {
  VariableList var_list;
  block_t *parent = nullptr;
  std::vector<block_t *> children;
};

class BlockList {
  public:
    // block_t block;
    block_t *current = nullptr;
    block_t *create_new_block(); // create new block and set it 'current'
    block_t *escape_block();
    block_t *get_current(); 
    VariableList *get_varlist();
    var_t   *lookup_var(std::string);
};

struct func_t {
  std::string name;
  Type *ret_type;
  std::vector<Type *> args_type;
  std::vector<llvm::Type *> llvm_args_type;
  std::vector<std::string> args_name;
  std::stack<bool> br_list;
  llvm::Function *llvm_function;
  BlockList block_list;
};

class FunctionList {
  private:
    std::vector<func_t> func_list;
  public:
    void add(func_t);
    func_t *get(std::string);
};
