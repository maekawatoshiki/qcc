#pragma once

#include "common.hpp"
#include "token.hpp"
#include "ast.hpp"


class Parser {
  private:
    Token token;
  public:
    int run(Token);

    AST_vec eval();
    AST *statement();

    bool is_function_def();
    bool is_function_proto();
    AST *make_function();
    AST *make_function_proto();
    AST *make_return();

    Type *skip_type();
    int skip_asterisk();

    std::map<std::string, int> op_prec;
    int get_op_prec(std::string);
    AST *expr_entry();
    AST *expr_index();
    AST *expr_dot();
    AST *expr_unary();
    AST *expr_primary();
    AST *expr_array();
    AST *expr_rhs(int, AST *);
};
