#pragma once

#include "common.hpp"

struct struct_t {
  std::string name;
  std::vector<std::string> members_name;
  llvm::StructType *llvm_struct;
};

class StructList {
  private:
    std::vector<struct_t> struct_list;
  public:
    void add(struct_t);
    void add(std::string name, std::vector<std::string>, llvm::StructType *);
    struct_t *get(std::string);
};
