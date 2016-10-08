#include "token.hpp"

token_t Token::get() {
  return token[pos];
}

token_t Token::next() {
  return token[pos++];
}

void Token::add_ident_tok  (std::string val, int line) {
  token.push_back((token_t) { TOK_TYPE_IDENT, val, line });
}
void Token::add_symbol_tok (std::string val, int line) {
  token.push_back((token_t) { TOK_TYPE_SYMBOL, val, line });
}
void Token::add_number_tok (std::string val, int line) {
  token.push_back((token_t) { TOK_TYPE_NUMBER, val, line });
}
void Token::add_string_tok (std::string val, int line) {
  token.push_back((token_t) { TOK_TYPE_STRING, val, line });
}
void Token::add_char_tok   (std::string val, int line) {
  token.push_back((token_t) { TOK_TYPE_CHAR, val, line });
}

void Token::show() {
  for(auto tok : token) 
    std::cout << tok.line << "L(" << tok.type << ") : " << tok.val << std::endl;
}
