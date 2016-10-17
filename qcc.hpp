#pragma once

#include "common.hpp"
#include "pp.hpp"
#include "lexer.hpp"
#include "parse.hpp"
#include "codegen.hpp"

class QCC {
  private:
    Preprocessor PP;
    Token token;
    Lexer LEX;
    Parser PARSE;
    Codegen CODEGEN;
  public:
    int run(std::string);
};
