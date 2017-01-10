#pragma once

#include "common.hpp"

enum {
  TOK_TYPE_IDENT,
  TOK_TYPE_SYMBOL,
  TOK_TYPE_NUMBER,
  TOK_TYPE_STRING,
  TOK_TYPE_CHAR,
  TOK_TYPE_NEWLINE,
  TOK_TYPE_END,
};

struct token_t {
  token_t(int _type, std::string _val = "", int _line = 0, bool _space = false):
    type(_type), val(_val), line(_line), space(_space) { };
  int type;
  std::string val;
  int line;
  bool space;
};

class Token {
  public:
    Token(std::vector<token_t> _token):token(_token) {};
    Token() {};

    std::vector<token_t> token;
    int pos = 0;

    void add_ident_tok  (std::string, int, bool = false);
    void add_symbol_tok (std::string, int, bool = false);
    void add_number_tok (std::string, int, bool = false);
    void add_string_tok (std::string, int, bool = false);
    void add_char_tok   (std::string, int, bool = false);
    void add_newline_tok();
    void add_end_tok    ();

    void seek(int);
    bool is_end();
    token_t get();
    token_t get(int skip); // pos + skip
    token_t next();

    bool is(std::string);
    void skip();
    bool skip(std::string);
    bool expect_skip(std::string);
    void prev();

    void show();
};
