#pragma once

#include "common.hpp"

enum {
  TY_VOID,
  TY_INT,
  TY_INT64,
  TY_CHAR,
  TY_FLOAT,
  TY_DOUBLE,
  TY_PTR,
  TY_USER,
};

struct type_t {
  type_t(int ty=TY_INT): type(ty) {};
  type_t(std::string ty): type(TY_USER) { user_type = ty; };
  int type;
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

    type_t &get();
    void change(int);
    void change(std::string);
    void change(Type *);
    void change(int, Type *);
    bool is_ptr();
    bool eql(int);
    bool eql(std::string);
    bool eql(Type *);
};

typedef std::vector<Type *> Type_vec;

namespace TypeTool {
  Type *to_type(std::string);
  std::string to_string(Type *);
};
