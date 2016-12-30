#pragma once

#include "common.hpp"

struct struct_t {
  std::string name;
  std::vector<std::string> members_name;
  llvm::StructType *llvm_struct;
};

struct union_elem_t {
  union_elem_t(std::string _s, llvm::Type *_t):
    name(_s), type(_t) {};
  std::string name;
  llvm::Type *type;
};

struct union_t {
  std::string name;
  std::vector<union_elem_t> members;
  llvm::StructType *llvm_union;
};

class StructList {
  private:
    std::vector<struct_t> struct_list;
  public:
    void add(struct_t);
    void add(std::string name, std::vector<std::string>, llvm::StructType *);
    struct_t *get(std::string);
};

class UnionList {
  private:
    std::vector<union_t> union_list;
  public:
    void add(union_t);
    void add(std::string name, std::vector<union_elem_t>, llvm::StructType *);
    union_t *get(std::string);
};
