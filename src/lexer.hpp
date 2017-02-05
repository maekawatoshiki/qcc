#pragma once

#include "common.hpp"
#include "token.hpp"

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
  std::string name;
  int type;
  Token rep;
  std::vector<std::string> args; // for function like macro
};

extern std::map<std::string, define_t> define_map;

class Lexer {
  private:
    int cur_line = 1;
    bool space = false; // means a leading space
    Token token;
    std::string line;
    std::ifstream ifs_src;
    bool comment = false;
    std::vector<token_t> buffer;

    token_t read_token();

    token_t tok_number();
    token_t tok_ident ();
    token_t tok_string();
    token_t tok_char  ();
    token_t tok_symbol();
    void skip_line    ();

    char replace_escape();

    // preprocessor
    std::vector<std::string> default_include_path = {
      "./include/",
      "/include/",
      "/usr/include/",
      "/usr/include/linux/",
      "/usr/include/x86_64-linux-gnu/",
      ""
    };
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

    void replace_macro(std::string macro_name);
    void replace_macro_object(define_t);
    void replace_macro_funclike(define_t);
    void subst_macro(Token &, Token args, define_t);
    bool is_defined(std::string);
    std::string stringize(std::vector<token_t> &);
    void add_macro_object(std::string name, std::vector<token_t> rep);
    void add_macro_funclike(std::string name, std::vector<std::string> args_name, std::vector<token_t> rep);
  public:
    Token run(std::string);
};
