#ifndef __QCC_TYPE__
#define __QCC_TYPE__

#include "common.hpp"

enum TypeKind {
  KIND_VOID,
  KIND_CHAR,
  KIND_SHORT,
  KIND_INT,
  KIND_LONG,
  KIND_LONG_LONG,
  KIND_ARRAY,
  KIND_PTR,
  KIND_STRUCT,
  KIND_UNION,
  KIND_ENUM,
  KIND_FUNC_PTR,
};

class Type {
    bool t_signed;
    int kind, size;
    Type *elem_type;
    std::string name;
    llvm::Type *llvm_type;
  public:
    Type(TypeKind, bool);
    Type(TypeKind, int=-1/*ary size(-1 is [])*/, Type * = nullptr);
    Type(TypeKind, Type *); // ptr
    Type(TypeKind, std::string, llvm::Type *); // struct union enum
    Type(TypeKind, llvm::Type *); // func ptr

    Type *get();
    Type *get_pointer_to();
    llvm::Type *to_llvm_type();
};

#endif
