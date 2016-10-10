#pragma once

#include "common.hpp"
#include "token.hpp"
#include "ast.hpp"


class Parser {
  private:
    Token token;
  public:
    int run(Token);

    int eval();

    bool is_function_def();
    bool is_function_proto();
    AST *make_function();

    Type *skip_type();
    int skip_asterisk();
};
