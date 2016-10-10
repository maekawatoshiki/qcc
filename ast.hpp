#pragma once

#include "common.hpp"
#include "type.hpp"

enum {
  AST_FUNCTION_PROTO,
  AST_FUNCTION_DEF,
};

class AST {
  public:
    virtual int get_type() = 0;
};

typedef std::vector<AST *> AST_vec;

class FunctionProtoAST : public AST {
  public:
    virtual int get_type() const { return AST_FUNCTION_PROTO; };
    FunctionProtoAST(std::string func_name, Type_vec argments_type);
};

struct argment_t {
  Type *type;
  std::string name;
};

class FunctionDefAST : public AST {
  public:
    virtual int get_type() const { return AST_FUNCTION_DEF; };
    FunctionDefAST(std::string func_name, std::vector<argment_t *> arguemts, AST_vec body);
};
