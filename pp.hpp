#pragma once

#include "common.hpp"
#include "token.hpp"

class Preprocessor {
  private:
    std::string default_include_path = "./include/";
    std::map<std::string, std::vector<token_t> > define_map;
    Token token;

    void read_include();
    void read_define();
  public:
    Token run(Token);
};
