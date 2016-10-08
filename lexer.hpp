#pragma once

#include "common.hpp"
#include "token.hpp"

class Lexer {
  public:
    Token run(std::string);
};
