#pragma once

#include "common.hpp"
#include "type.hpp"

enum {
  AST_FUNCTION_PROTO,
  AST_FUNCTION_DEF,
  AST_FUNCTION_CALL,
  AST_VAR_DECLARATION,
  AST_RETURN,
  AST_NUMBER,
  AST_STRING,
};

class AST {
  public:
    virtual int get_type() const = 0;
};

typedef std::vector<AST *> AST_vec;

class FunctionProtoAST : public AST {
  public:
    std::string name;
    Type *ret_type;
    Type_vec args;
    virtual int get_type() const { return AST_FUNCTION_PROTO; };
    FunctionProtoAST(std::string func_name, Type *, Type_vec argments_type);
};

struct argument_t {
  argument_t(Type *ty, std::string nm):type(ty), name(nm) {};
  Type *type;
  std::string name;
};

class FunctionDefAST : public AST {
  public:
    std::string name;
    Type *ret_type;
    std::vector<argument_t *> args;
    AST_vec body;
    virtual int get_type() const { return AST_FUNCTION_DEF; };
    FunctionDefAST(std::string, Type *, std::vector<argument_t *>, AST_vec);
};

class FunctionCallAST : public AST {
  public:
    std::string name;
    AST_vec args;
    virtual int get_type() const { return AST_FUNCTION_CALL; };
    FunctionCallAST(std::string name, AST_vec args);
};

typedef argument_t declarator_t;
class VarDeclarationAST : public AST {
  public: 
    std::vector<declarator_t *> decls;
    virtual int get_type() const { return AST_VAR_DECLARATION; };
    VarDeclarationAST(std::vector<declarator_t *>);
};

class ReturnAST : public AST {
  public:
    AST *expr;
    virtual int get_type() const { return AST_RETURN; };
    ReturnAST(AST *);
};

class NumberAST : public AST {
  public:
    int number;
    virtual int get_type() const { return AST_NUMBER; };
    NumberAST(int);
};

class StringAST : public AST {
  public:
    std::string str;
    virtual int get_type() const { return AST_STRING; }
    StringAST(std::string);
};
