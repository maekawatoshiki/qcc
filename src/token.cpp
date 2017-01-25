#include "token.hpp"

token_t Token::get() {
  return token[pos];
}

token_t Token::get(int skip) {
  return token[pos + skip];
}

token_t Token::next() {
  return token[pos++];
}

void Token::seek(int pos) {
  this->pos = pos;
}

bool Token::is_end() {
  return !(pos < token.size());
}

void Token::add_ident_tok  (std::string val, int line, bool space) {
  token.push_back((token_t) { TOK_TYPE_IDENT, val, line, space });
}
void Token::add_symbol_tok (std::string val, int line, bool space) {
  token.push_back((token_t) { TOK_TYPE_SYMBOL, val, line, space });
}
void Token::add_number_tok (std::string val, int line, bool space) {
  token.push_back((token_t) { TOK_TYPE_NUMBER, val, line, space });
}
void Token::add_string_tok (std::string val, int line, bool space) {
  token.push_back((token_t) { TOK_TYPE_STRING, val, line, space });
}
void Token::add_char_tok   (std::string val, int line, bool space) {
  token.push_back((token_t) { TOK_TYPE_CHAR, val, line, space });
}
void Token::add_newline_tok () {
  token.push_back((token_t) { TOK_TYPE_NEWLINE });
}
void Token::add_end_tok    () {
  token.push_back((token_t) { TOK_TYPE_END });
}

bool Token::is(std::string str) { return get().val == str && get().type != TOK_TYPE_STRING && get().type != TOK_TYPE_CHAR; }
void Token::skip() { pos++; }
bool Token::skip(std::string str) { 
  if(str == get().val && get().type != TOK_TYPE_STRING && get().type != TOK_TYPE_CHAR) {
    pos++; 
    if(pos > token.size()) error("error: program is reached EOF");
    return true;
  } else return false; 
}
bool Token::expect_skip(std::string str) { 
  if(str == get().val && get().type != TOK_TYPE_STRING && get().type != TOK_TYPE_CHAR) {
    pos++; 
    if(pos > token.size()) error("error: program is reached EOF");
    return true;
  } else {
    puts("surrounding tokens:");
    for(int p = pos > 3 ? -3 : 0; p < 3 && !is_end(); p++) 
      printf("%s", ((get(p).space ? " " : "") + get(p).val).c_str());
    puts("");
    error("error(%d): expected '%s'", get().line, str.c_str());
  }
  return false;
}
void Token::prev() { pos--; }

void Token::show() {
  for(auto tok : token) 
    std::cout << tok.line << "L(" << tok.type << ")(" << tok.space <<") : " << tok.val << std::endl;
}
