#pragma once

#include "common.hpp"
#include "token.hpp"


enum {
  DEFINE_MACRO,
  DEFINE_FUNCLIKE_MACRO
};
struct define_t {
  int type;
  std::vector<token_t> rep;
  std::vector<std::string> args; // for function like macro
};

class Preprocessor {
  private:
    std::string default_include_path = "./include/";
    std::map<std::string, define_t> define_map;
    Token token, new_token;
    std::stack<bool> cond_stack; 

    bool read_expr_line();
    void read_include();
    void read_define();
    void read_undef();
    void do_read_if(bool);
    void read_if();
    void read_ifdef();
    void read_ifndef();
    void read_else();
    void skip_cond_include();

    void skip_this_line();

    void replace_macro(); // replace current token to macro

    void add_define_macro(std::string macro_name, std::vector<token_t> rep);
    void add_define_funclike_macro(std::string macro_name, 
        std::vector<std::string> args_name, std::vector<token_t> rep);
  public:
    Token run(Token);
};
