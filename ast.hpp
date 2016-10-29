#pragma once

#include "common.hpp"
#include "type.hpp"

enum {
  AST_FUNCTION_PROTO,
  AST_FUNCTION_DEF,
  AST_FUNCTION_CALL,
  AST_BLOCK,
  AST_VAR_DECLARATION,
  AST_STRUCT_DECLARATION,
  AST_UNARY,
  AST_BINARY,
  AST_DOT,
  AST_INDEX,
  AST_VARIABLE,
  AST_IF,
  AST_WHILE,
  AST_FOR,
  AST_ASGMT,
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
    AST *body;
    virtual int get_type() const { return AST_FUNCTION_DEF; };
    FunctionDefAST(std::string, Type *, std::vector<argument_t *>, AST *);
};

class FunctionCallAST : public AST {
  public:
    std::string name;
    AST_vec args;
    virtual int get_type() const { return AST_FUNCTION_CALL; };
    FunctionCallAST(std::string name, AST_vec args);
};

class BlockAST : public AST {
  public:
    AST_vec body;
    virtual int get_type() const { return AST_BLOCK; };
    BlockAST(AST_vec);
};

class UnaryAST : public AST {
  public:
    AST *expr;
    std::string op;
    virtual int get_type() const { return AST_UNARY; };
    UnaryAST(std::string, AST *);
};

class BinaryAST : public AST {
  public:
    std::string op;
    AST *lhs, *rhs;
    virtual int get_type() const { return AST_BINARY; };
    BinaryAST(std::string, AST *, AST *);
};

class DotOpAST : public AST {
  public:
    AST *lhs, *rhs;
    bool is_arrow = false;
    virtual int get_type() const { return AST_DOT; };
    DotOpAST(AST *, AST *, bool = false);
};

struct declarator_t {
  declarator_t(Type *ty, std::string nm, AST *init_exp = nullptr):type(ty), name(nm), init_expr(init_exp) {};
  Type *type;
  std::string name;
  AST *init_expr = nullptr;
};
class VarDeclarationAST : public AST {
  public: 
    std::vector<declarator_t *> decls;
    virtual int get_type() const { return AST_VAR_DECLARATION; };
    VarDeclarationAST(std::vector<declarator_t *>);
};

class StructDeclarationAST : public AST {
  public:
    std::string name;
    AST *decls;
    virtual int get_type() const { return AST_STRUCT_DECLARATION; };
    StructDeclarationAST(std::string name, AST *);
};

class VariableAST : public AST {
  public:
    std::string name;
    virtual int get_type() const { return AST_VARIABLE; };
    VariableAST(std::string);
};

class IndexAST : public AST {
  public:
   AST *ary, *idx;
   virtual int get_type() const { return AST_INDEX; };
   IndexAST(AST *, AST *);
};

class IfAST : public AST {
  public:
    AST *cond, *b_then, *b_else;
    virtual int get_type() const { return AST_IF; };
    IfAST(AST *, AST *, AST * = nullptr);
};

class WhileAST : public AST {
  public:
    AST *cond, *body;
    virtual int get_type() const { return AST_WHILE; };
    WhileAST(AST *, AST *);
};

class ForAST : public AST {
  public:
    AST *init, *cond, *reinit, *body;
    virtual int get_type() const { return AST_FOR; };
    ForAST(AST *, AST *, AST *, AST *);
};

class AsgmtAST : public AST {
  public:
    AST *dst, *src;
    virtual int get_type() const { return AST_ASGMT; };
    AsgmtAST(AST *, AST *);
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
