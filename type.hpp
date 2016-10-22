#pragma once

#include "common.hpp"

enum {
  TY_VOID,
  TY_INT,
  TY_INT64,
  TY_CHAR,
  TY_FLOAT,
  TY_DOUBLE,
  TY_VARARG,
  TY_PTR,
  TY_ARRAY,
  TY_TYPEDEF,
  TY_STRUCT,
  TY_STRUCT_ANONYMOUS,
  TY_UNION,
  TY_UNION_ANONUMOUS,
};

struct type_t {
  type_t(int ty=TY_INT): type(ty) {}; // normal type
  type_t(int ary, int size): type(ary), ary_size(size) {}; // array
  type_t(std::string ty): type(TY_TYPEDEF) { user_type = ty; }; // typedef type
  type_t(int su, std::string ty): type(su) { user_type = ty; }; // struct or union type
  int type, ary_size;
  std::string user_type;
};

class Type {
  private:
    type_t type;
  public:
    Type *next;
    Type() {};
    Type(int ty): type(ty) {};
    Type(std::string ty): type(ty) {};
    Type(Type *ty): type(ty->type), next(ty->next) {};
    Type(int ty, Type *type): type(ty), next(type) {};
    Type(int ty, int sz, Type *type): type(ty, sz), next(type) {};

    type_t &get();
    void change(int);
    void change(std::string);
    void change(Type *);
    void change(int, Type *);
    bool is_ptr();
    bool eql(int);
    bool eql(std::string);
    bool eql(Type *);

    std::string to_string();
};

typedef std::vector<Type *> Type_vec;

namespace TypeTool {
  Type *to_type(std::string);
  std::string to_string(Type *);
};
