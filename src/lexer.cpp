#include "lexer.hpp"
#include "token.hpp"

Token Lexer::run(std::string file_name) {
  ifs_src.open(file_name);
  if(ifs_src.fail()) error("file not found %s", file_name.c_str());
  while(1) {
    auto t = read_token();
    if(t.type == TOK_TYPE_END) break;
    std::cout << t.val << std::endl;
  }
  puts("END");

  cur_line = 1;
  ifs_src.close();


  return token;
}

token_t Lexer::read_token() {
  char c = ifs_src.get();
  if(ifs_src.eof()) return token_t(TOK_TYPE_END);
  if(c == '/') {
    c = ifs_src.get();
    if(c == '*') {
      for(; !ifs_src.eof();) {
        if(c == '*' && (c = ifs_src.get()) == '/') 
          break;
        else c = ifs_src.get();
      }
    } else if(c == '/') {
      for(; c != '\n' && !ifs_src.eof(); c = ifs_src.get()) {};
    }
    return read_token();
  } else {
    ifs_src.unget();
    if(isdigit(c)) {                                  return tok_number();
    } else if(c == '_' || isalpha(c)) {               return tok_ident();
    } else if(isblank(c)) { ifs_src.get();            return read_token();
    } else if(c == '\n') { cur_line++; ifs_src.get(); return read_token();
    } else if(c == '\\') { ifs_src.get();             return read_token();
    } else if(c == '\"') {                            return tok_string();
    } else if(c == '\'') {                            return tok_char();
    } else                                            return tok_symbol();
  }
}

token_t Lexer::tok_number() {
  std::string number;
  char last = 0;
  for(;;) {
    char c = ifs_src.get();
    bool is_float = strchr("eEpP", last) && strchr("+-", c);
    if(!isdigit(c) && !isalpha(c) && c != '.' && !is_float) break;
    number += c;
    last = c;
  }
  ifs_src.unget();
  return token_t(TOK_TYPE_NUMBER, number, cur_line);
}
token_t Lexer::tok_ident() {
  std::string ident;
  char c = ifs_src.get();
  for(; isalpha(c) || isdigit(c) || 
      c == '_'; c = ifs_src.get())
    ident += c;
  ifs_src.unget();
  return token_t(TOK_TYPE_IDENT, ident, cur_line);
}
token_t Lexer::tok_string() {
  std::string content;
  ifs_src.get(); // "
  for(char c = ifs_src.get(); c != '\"'; c = ifs_src.get()) 
    content += (c == '\\') ? replace_escape() : c;
  return token_t(TOK_TYPE_STRING, content, cur_line);
}
token_t Lexer::tok_char() {
  ifs_src.get(); std::string ch;
  char c = ifs_src.get();
  ch = (c == '\\') ? replace_escape() : c;
  ifs_src.get();
  return token_t(TOK_TYPE_CHAR, ch, cur_line);
}
token_t Lexer::tok_symbol() {
  char c = ifs_src.get();
  std::string op; op = c;
  char cn = ifs_src.get();
  if(               
      (c == '+' && cn == '+') ||
      (c == '-' && cn == '-') ||
      (c == '&' && cn == '&') ||
      (c == '|' && cn == '|') ||
      (c == '-' && cn == '>') ||
      (c == '<' && cn == '<') ||
      (c == '>' && cn == '>') ||
      (c == '.' && cn == '.') )
    op += cn;
  else {
    ifs_src.unget();
    c = cn;
  }

  cn = ifs_src.get();
  if(c == '.' && cn == '.') op += cn; // variable arguments '...'
  else if(/*TODO: fix=*/c != ']' && /*TODO: fix*/cn == '=') op += cn; // compare 'X=' e.g. <= ==
  else ifs_src.unget();
  return token_t(TOK_TYPE_SYMBOL, op, cur_line);
}

void Lexer::skip_line(std::string::iterator &pos) {
  puts("HRE");
  while(pos != line.end()) pos++;
}

char Lexer::replace_escape() {
  char c = ifs_src.get();
  switch(c) {
    case '\'': case '"': case '?': case '\\':
      return c;
    case 'a': return '\a';
    case 'b': return '\b';
    case 'f': return '\f';
    case 'n': return '\n';
    case 'r': return '\r';
    case 't': return '\t';
    case 'v': return '\v';
  }
  return c;
}
