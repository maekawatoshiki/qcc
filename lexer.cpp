#include "lexer.hpp"
#include "token.hpp"

Token Lexer::run(std::string source) {
  Token token;
  int cur_line = 1;
  bool space = false;

  for(int i = 0; i < source.size(); i++) {
    if(isdigit(source[i])) {
      std::string number;
      for(; isdigit(source[i]) || source[i] == '.'; i++)
        number += source[i];
      --i; token.add_number_tok(number, cur_line, space);
      space = false;
    } else if(source[i] == '_' || isalpha(source[i])) {
      std::string ident;
      for(; isalpha(source[i]) || isdigit(source[i]) || 
          source[i] == '_'; i++)
        ident += source[i];
      --i; token.add_ident_tok(ident, cur_line, space);
      space = false;
    } else if(isblank(source[i])) { // skip
      space = true;
    } else if(source[i] == '/' && source[i+1] == '/') {
      for(; source[i] != '\n'; i++) {};
    } else if(source[i] == '/' && source[i+1] == '*') {
      for(; !(source[i] == '*' && source[i+1] == '/'); i++) { ; }
      i++;
    } else if(source[i] == '\"') {
      std::string content;
      for(i++; source[i] != '\"' && i < source.size(); i++) {
        content += source[i];
        if(source[i] == '\\') 
          content += source[++i];
      }
      token.add_string_tok(content, cur_line, space);
      space = false;
    } else if(source[i] == '\'') {
      if(source[i+1] == '\\') { // TODO: implemet escape sequence
      } else {
        i++; std::string ch;ch=source[i];
        token.add_char_tok(ch, cur_line, space);
        space = false;
        i++; // '
      }
    } else if(source[i] == '\n') { // skip // TODO: windows support?
      cur_line++;
    } else {
      std::string op; op=source[i];
      if(source[i+1] == '=' ||
          (source[i] == '+' && source[i+1] == '+') ||
          (source[i] == '-' && source[i+1] == '-') ||
          (source[i] == '&' && source[i+1] == '&') ||
          (source[i] == '|' && source[i+1] == '|') ||
          (source[i] == '-' && source[i+1] == '>') ||
          (source[i] == '.' && source[i+1] == '.') )
        op += source[++i];
      if(source[i+1] == '.') op += source[++i];
      token.add_symbol_tok(op, cur_line, space);
      space = false;
    }
  } 

  token.add_end_tok();

  return token;
}
