#pragma once

#include "common.hpp"
#include "token.hpp"

class Lexer {
  private:
    int cur_line = 1;
    bool space = false; // means a leading space
    Token token;
    std::string line;
    std::ifstream ifs_src;
    bool comment = false;

    token_t read_token();

    token_t tok_number();
    token_t tok_ident ();
    token_t tok_string();
    token_t tok_char  ();
    token_t tok_symbol();
    void skip_line (std::string::iterator &);

    char replace_escape();
  public:
    Token run(std::string);
};
