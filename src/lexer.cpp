#include "lexer.hpp"
#include "token.hpp"

Token Lexer::run(std::string file_name) {
  std::ifstream ifs_src(file_name);
  if(ifs_src.fail()) error("file not found %s", file_name.c_str());

  cur_line = 1;
  space = false;

  bool comment = false;
  while(std::getline(ifs_src, line)) {
    bool nl = false;
    for(auto pos = line.begin(); pos < line.end(); ++pos) {
      if(comment || (*pos == '/' && *(pos+1) == '*')) {
        for(; pos != line.end() && !(*pos == '*' && *(pos+1) == '/'); pos++) {};
        if(pos!=line.end() && *pos == '*') {
          pos++; // '/'
          comment = false;
        } else comment = true;
      } else if(isdigit(*pos))              tok_number(pos);
      else if(*pos == '_' || isalpha(*pos)) tok_ident(pos);
      else if(isblank(*pos))                space = true; // skip
      else if(*pos == '\\') {               skip_line(pos); nl = true; }
      else if(*pos == '/' && *(pos+1) == '/') {
        for(; pos != line.end(); pos++) {}; 
      } else if(*pos == '\"')               tok_string(pos);
      else if(*pos == '\'')                 tok_char(pos);
      else                                  tok_symbol(pos);
    }
    cur_line++;
    if(!nl) token.add_newline_tok();
  }
  ifs_src.close();

  token.add_end_tok();

  return token;
}

void Lexer::tok_number(std::string::iterator &pos) {
  std::string number;
  for(;;) {
    char c = *pos;
    bool is_float = strchr("eEpP", *(pos-1)) && strchr("+-", c);
    if(!isdigit(c) && !isalpha(c) && c != '.' && !is_float) break;
    number += *pos++;
  }
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
  for(pos++; *pos != '\"'; pos++) 
    content += (*pos == '\\') ? replace_escape(pos) : *pos;

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
  if(               
      (*pos == '+' && *(pos+1) == '+') ||
      (*pos == '-' && *(pos+1) == '-') ||
      (*pos == '&' && *(pos+1) == '&') ||
      (*pos == '|' && *(pos+1) == '|') ||
      (*pos == '-' && *(pos+1) == '>') ||
      (*pos == '<' && *(pos+1) == '<') ||
      (*pos == '>' && *(pos+1) == '>') ||
      (*pos == '.' && *(pos+1) == '.') )
    op += *++pos;
  if(*pos == '.' && *(pos+1) == '.') op += *++pos; // variable arguments '...'
  else if(*(pos+1) == '=') op += *++pos; // compare 'X=' e.g. <= ==
  token.add_symbol_tok(op, cur_line, space);
  space = false;
}

void Lexer::skip_line(std::string::iterator &pos) {
  puts("HRE");
  while(pos != line.end()) pos++;
}

char Lexer::replace_escape(std::string::iterator &pos) {
  char c = *++pos;
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
