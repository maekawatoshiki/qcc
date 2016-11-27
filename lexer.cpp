#include "lexer.hpp"
#include "token.hpp"

Token Lexer::run(std::string source) {
  cur_line = 1;
  space = false;

  for(auto pos = source.begin(); pos != source.end(); ++pos) {
    if(isdigit(*pos))                     tok_number(pos);
    else if(*pos == '_' || isalpha(*pos)) tok_ident(pos);
    else if(isblank(*pos))                space = true; // skip
    else if(*pos == '\\')                 skip_line(pos);
    else if(*pos == '/' && *(pos+1) == '/') {
      for(; *pos != '\n'; pos++) {};
    } else if(*pos == '/' && *(pos+1) == '*') {
      for(; !(*pos == '*' && *(pos+1) == '/'); pos++) { ; }
      pos++;
    } else if(*pos == '\"')               tok_string(pos);
    else if(*pos == '\'')                 tok_char(pos);
    else if(*pos == '\n')                 skip_line(pos), cur_line++;
    else                                  tok_symbol(pos);
  } 

  token.add_end_tok();

  return token;
}

void Lexer::tok_number(std::string::iterator &pos) {
  std::string number;
  for(; isdigit(*pos) || *pos == '.'; pos++)
    number += *pos;
  --pos; token.add_number_tok(number, cur_line, space);
  space = false;
}
void Lexer::tok_ident(std::string::iterator &pos) {
  std::string ident;
  for(; isalpha(*pos) || isdigit(*pos) || 
      *pos == '_'; pos++)
    ident += *pos;
  --pos; token.add_ident_tok(ident, cur_line, space);
  space = false;
}
void Lexer::tok_string(std::string::iterator &pos) {
  std::string content;
  for(pos++; *pos != '\"'; pos++) {
    content += *pos;
    if(*pos == '\\') 
      content += *++pos;
  }
  token.add_string_tok(content, cur_line, space);
  space = false;
}
void Lexer::tok_char(std::string::iterator &pos) {
  if(*(pos+1) == '\\') { // TODO: implemet escape sequence
  } else {
    pos++; std::string ch;ch=*pos++;
    token.add_char_tok(ch, cur_line, space);
    space = false;
  }
}
void Lexer::tok_symbol(std::string::iterator &pos) {
  std::string op; op = *pos;
  if(                 *(pos+1) == '='  ||
      (*pos == '+' && *(pos+1) == '+') ||
      (*pos == '-' && *(pos+1) == '-') ||
      (*pos == '&' && *(pos+1) == '&') ||
      (*pos == '|' && *(pos+1) == '|') ||
      (*pos == '-' && *(pos+1) == '>') ||
      (*pos == '.' && *(pos+1) == '.') )
    op += *++pos;
  if(*(pos+1) == '.') op += *++pos; // variable arguments '...'
  token.add_symbol_tok(op, cur_line, space);
  space = false;
}

void Lexer::skip_line(std::string::iterator &pos) {
  while(*pos != '\n') pos++;
}
