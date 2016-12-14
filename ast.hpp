#pragma once

#include "common.hpp"

enum {
  AST_FUNCTION_PROTO,
  AST_FUNCTION_DEF,
  AST_FUNCTION_CALL,
  AST_BLOCK,
  AST_VAR_DECLARATION,
  AST_TYPEDEF,
  AST_ARRAY,
  AST_UNARY,
  AST_BINARY,
  AST_TERNARY,
  AST_DOT,
  AST_INDEX,
  AST_VARIABLE,
  AST_BREAK,
  AST_CONTINUE,
  AST_IF,
  AST_WHILE,
  AST_FOR,
  AST_ASGMT,
  AST_RETURN,
  AST_SIZEOF,
  AST_NUMBER,
  AST_STRING,
};

class AST {
  public:
    virtual int get_type() const = 0;
};

typedef std::vector<AST *> AST_vec;

struct argument_t {
  argument_t(llvm::Type *ty, std::string nm):type(ty), name(nm) {};
  llvm::Type *type;
  std::string name;
};

class FunctionProtoAST : public AST {
  public:
    std::string name;
    llvm::Type *ret_type;
    std::vector<argument_t *> args;
    virtual int get_type() const { return AST_FUNCTION_PROTO; };
    FunctionProtoAST(std::string func_name, llvm::Type *, std::vector<argument_t *>argments_type);
};

class FunctionDefAST : public AST {
  public:
    std::string name;
    llvm::Type *ret_type;
    std::vector<argument_t *> args;
    AST_vec body;
    virtual int get_type() const { return AST_FUNCTION_DEF; };
    FunctionDefAST(std::string, llvm::Type *, std::vector<argument_t *>, AST_vec);
};

class FunctionCallAST : public AST {
  public:
    AST *callee;
    AST_vec args;
    virtual int get_type() const { return AST_FUNCTION_CALL; };
    FunctionCallAST(AST *callee, AST_vec args);
};

class BlockAST : public AST {
  public:
    AST_vec body;
    virtual int get_type() const { return AST_BLOCK; };
    BlockAST(AST_vec);
};

class ArrayAST : public AST {
  public:
    AST_vec elems;
    virtual int get_type() const { return AST_ARRAY; };
    ArrayAST(AST_vec);
};

class UnaryAST : public AST {
  public:
    AST *expr;
    std::string op;
    bool postfix = false;
    virtual int get_type() const { return AST_UNARY; };
    UnaryAST(std::string, AST *, bool = false);
};

class BinaryAST : public AST {
  public:
    std::string op;
    AST *lhs, *rhs;
    virtual int get_type() const { return AST_BINARY; };
    BinaryAST(std::string, AST *, AST *);
};

class TernaryAST : public AST {
  public:
    AST *cond, *then_expr, *else_expr;
    virtual int get_type() const { return AST_TERNARY; };
    TernaryAST(AST *, AST *, AST *);
};

class DotOpAST : public AST {
  public:
    AST *lhs, *rhs;
    bool is_arrow = false;
    virtual int get_type() const { return AST_DOT; };
    DotOpAST(AST *, AST *, bool = false);
};

struct declarator_t {
  declarator_t(llvm::Type *ty, std::string nm, AST *init_exp = nullptr):type(ty), name(nm), init_expr(init_exp) {};
  llvm::Type *type;
  std::string name;
  AST *init_expr = nullptr;
};
class VarDeclarationAST : public AST {
  public: 
    std::vector<declarator_t *> decls;
    virtual int get_type() const { return AST_VAR_DECLARATION; };
    VarDeclarationAST(std::vector<declarator_t *>);
};

class TypedefAST : public AST {
  public:
    llvm::Type *from;
    std::string to;
    virtual int get_type() const { return AST_TYPEDEF; };
    TypedefAST(llvm::Type *, std::string);
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

class BreakAST : public AST {
  public:
    virtual int get_type() const { return AST_BREAK; };
};

class ContinueAST : public AST {
  public:
    virtual int get_type() const { return AST_CONTINUE; };
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

class SizeofAST : public AST {
  public:
    llvm::Type *type;
    virtual int get_type() const { return AST_SIZEOF; };
    SizeofAST(llvm::Type *);
};

class NumberAST : public AST {
  public:
    bool is_float = false;
    union {
      int i_number;
      double f_number;
    };
    virtual int get_type() const { return AST_NUMBER; };
    NumberAST(int);
    NumberAST(double);
};

class StringAST : public AST {
  public:
    std::string str;
    virtual int get_type() const { return AST_STRING; }
    StringAST(std::string);
};
