#include "lexer.hpp"
#include "token.hpp"

Token Lexer::run(std::string source) {
  Token token;
  int cur_line = 1;

  for(int i = 0; i < source.size(); i++) {
    if(isdigit(source[i])) {
      std::string number;
      for(; isdigit(source[i]) || source[i] == '.'; i++)
        number += source[i];
      --i; token.add_number_tok(number, cur_line);
    } else if(source[i] == '-' || isalpha(source[i])) {
      std::string ident;
      for(; isalpha(source[i]) || isdigit(source[i]) || 
          source[i] == '_'; i++)
        ident += source[i];
      --i; token.add_ident_tok(ident, cur_line);
    } else if(isblank(source[i])) { // skip
    } else if(source[i] == '/' && (source[i+1] == '/' ||
                                   source[i+1] == '*')) {
      i++; bool is_oneline=source[i] == '/';
      for(; is_oneline ? source[i] != '\n' : 
            (source[i] != '*' && source[i+1] != '/'); i++) { ; }
    } else if(source[i] == '\"') {
      std::string content;
      for(i++; source[i] != '\"' && i < source.size(); i++) {
        content += source[i];
        if(source[i] == '\\') 
          content += source[++i];
      }
      token.add_string_tok(content, cur_line);
    } else if(source[i] == '\'') {
      if(source[i+1] == '\\') { // TODO: implemet escape sequence
      } else {
        i++; std::string ch;ch=source[i];
        token.add_char_tok(ch, cur_line);
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
          (source[i] == '|' && source[i+1] == '|'))
        op += source[++i];
      token.add_symbol_tok(op, cur_line);
    }
  } 

  token.add_end_tok();

  return token;
}
