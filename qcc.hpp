#pragma once

#include "common.hpp"
#include "pp.hpp"
#include "lexer.hpp"
#include "parse.hpp"

class QCC {
  private:
    Preprocessor PP;
    Token token;
    Lexer LEX;
    Parser PARSE;
  public:
    int run(std::string);
};
