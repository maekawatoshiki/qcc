#pragma once

#include "common.hpp"
#include "token.hpp"
#include "ast.hpp"


class Parser {
  private:
    Token token;
  public:
    AST_vec run(Token);

    AST_vec eval();
    AST *statement();

    bool is_function_def();
    bool is_function_proto();
    bool is_var_declaration();
    AST *make_function();
    AST *make_function_proto();
    AST *make_var_declaration();
    AST *make_struct_declaration();
    AST *make_block();
    AST *make_if();
    AST *make_while();
    AST *make_for();
    AST *make_return();

    Type *skip_type_spec();
    Type *skip_declarator();
    int skip_pointer();
    std::vector<int> skip_array(); // .size() = number of [], elem = ary size

    std::map<std::string, int> op_prec;
    int get_op_prec(std::string);
    AST *expr_entry();
    AST *expr_asgmt();
    AST *expr_unary();
    AST *expr_dot();
    AST *expr_index();
    AST *expr_primary();
    AST *expr_array();
    AST *expr_rhs(int, AST *);
};
