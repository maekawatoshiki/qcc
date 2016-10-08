#pragma once

#include "common.hpp"
#include "pp.hpp"
#include "lexer.hpp"

class QCC {
  private:
    Preprocessor PP;
    Lexer LEX;
    Token token;
  public:
    int run(std::string);
};
