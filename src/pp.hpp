#pragma once

#include "common.hpp"
#include "token.hpp"
#include "ast.hpp"


enum {
  DEFINE_MACRO,
  DEFINE_FUNCLIKE_MACRO
};

struct macro_token_t {
  macro_token_t(token_t _token, bool _stringify = false):
    token(_token), stringify(_stringify) {};
  token_t token;
  bool stringify;
};

struct define_t {
  int type;
  std::vector<macro_token_t> rep;
  std::vector<std::string> args; // for function like macro
};

class Preprocessor {
  private:
    std::vector<std::string> default_include_path = {
      "./include/",
      "/include/",
      "/usr/include/",
      "/usr/include/linux/",
      "/usr/include/x86_64-linux-gnu/",
    };
    Token token, new_token;
    std::stack<bool> cond_stack; 

    void read_include();
    void read_define();
    void read_undef();
    void do_read_if(bool);
    void read_if();
    void read_elif();
    void read_ifdef();
    void read_ifndef();
    void read_else();
    void skip_cond_include();

    bool read_constexpr();
    Token read_expr_line();
    token_t read_defined_op();

    void skip_this_line();

    std::vector<token_t> replace_macro(); // replace current token to macro

    void add_define_macro(std::string macro_name, std::vector<macro_token_t> rep);
    void add_define_funclike_macro(std::string macro_name, 
        std::vector<std::string> args_name, std::vector<macro_token_t> rep);
  public:
    std::map<std::string, define_t> define_map;
    Token run(Token);
};
