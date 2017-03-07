#include "lexer.hpp"
#include "token.hpp"
#include "parse.hpp"

std::map<std::string, macro_t> macro_map;
Token Lexer::run(std::string file_name) {
  ifs_src.open(file_name);
  if(ifs_src.fail()) error("file not found %s", file_name.c_str());
  while(1) {
    auto t = read_token();
    if(t.type == TOK_TYPE_END) break;
    if(t.val == "#") {
      t = read_token();
      auto &s = t.val;
           if(s == "include") read_include();
      else if(s == "define" ) read_define();
      else if(s == "undef"  ) read_undef();
      else if(s == "if"     ) read_if();
      else if(s == "ifdef"  ) read_ifdef();
      else if(s == "ifndef" ) read_ifndef();
      else if(s == "elif"   ) read_elif();
      else if(s == "else"   ) read_else();
      else if(s == "endif"  ) skip_line();
      else if(s == "error"  ) skip_line();
      else if(s == "warning") skip_line();
      else if(s == "pragma" ) skip_line();
      else error("PREPROCESSOR ERR '%s'", t.val.c_str());
    } else if(t.type == TOK_TYPE_IDENT && !t.hideset.count(t.val) && is_defined(t.val)) {
      replace_macro(t.val);
    } else if(t.type != TOK_TYPE_NEWLINE)
      token.token.push_back(t);
  }

  token.add_end_tok();
  cur_line = 1;
  ifs_src.close();
  // token.show();

  return token;
}

token_t Lexer::read_token() {
  if(buffer.size() > 0) {
    auto r = buffer.front();
    buffer.erase(buffer.begin());
    return r;
  }
  char c = ifs_src.get();
  if(ifs_src.eof()) return token_t(TOK_TYPE_END);
  if(c == '/') {
    c = ifs_src.get();
    if(c == '*') {
      char last = c;
      for(; !ifs_src.eof();) {
        c = ifs_src.get();
        if(last == '*' && c == '/')
          break;
        last = c;
      }
      return read_token();
    } else if(c == '/') {
      for(; c != '\n' && !ifs_src.eof(); c = ifs_src.get()) {};
      ifs_src.unget();
      return read_token();
    } else { ifs_src.unget(); c = '/'; }
  }
  {
    ifs_src.unget();
    if(isdigit(c)) {                                  return tok_number();
    } else if(c == '_' || isalpha(c)) {               return tok_ident();
    } else if(isblank(c)) { ifs_src.get(); auto t = read_token(); t.space = true; return t;
    } else if(c == '\n') { cur_line++; ifs_src.get(); return token_t(TOK_TYPE_NEWLINE);
    } else if(c == '\\') { while(ifs_src.get() != '\n'){}; cur_line++; return read_token();
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
  if(op == ".." && cn == '.') op += cn; // variadic arguments '...'
  else if(/*TODO: fix=*/op[0] != ']' && /*TODO: fix*/cn == '=') op += cn; // compare 'X=' e.g. <= ==
  else ifs_src.unget();
  return token_t(TOK_TYPE_SYMBOL, op, cur_line);
}

void Lexer::skip_line() {
  auto t = read_token();
  while(t.type != TOK_TYPE_NEWLINE && t.type != TOK_TYPE_END) t = read_token();
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

void Lexer::read_include() {
  std::string file_name;
  auto t = read_token();
  if(t.val == "<") {
    while((t = read_token()).val != ">") {
      // std::cout << t.val <<std::endl;
      file_name += t.val;
    }
  } else if(t.type == TOK_TYPE_STRING) {
    file_name = t.val;
  }
  std::function<std::string(int)> find_include_file = [&](size_t incl_n) -> std::string {
    if(incl_n == default_include_path.size()) { error("error: not found such file '%s'", file_name.c_str()); }
    std::ifstream ifs_src(default_include_path[incl_n] + file_name);
    if(!ifs_src) { return find_include_file(incl_n+1); }
    // std::cout << "NOW INCLUDING " << default_include_path[incl_n] + file_name << std::endl;getchar();
    return default_include_path[incl_n] + file_name;
  };
  Lexer lex; auto tok = lex.run(find_include_file(0));
  tok.token.pop_back(); // delete TOK_TYPE_END
  for(auto t : tok.token)
    token.token.push_back(t);
  // std::copy(token.token.begin(), token.token.end(), std::back_inserter(tok.token));
}

void Lexer::read_define() {
  std::string macro_name; bool funclike = false;
  std::vector<std::string> args_name;
  auto t = read_token();
  // DEBUG: std::cout << t.val << std::endl;getchar();
  if(t.type == TOK_TYPE_IDENT) {
    macro_name = t.val;
    t = read_token();
  }
  if(t.val == "(" && !t.space) {
    funclike = true;
    while(1) {
      t = read_token();
      if(t.val == ")") break;
      else args_name.push_back(t.val);
      t = read_token();
      if(t.val == ")") break;
      if(t.val != ",") error("error(%d): expected ','", t.line);
    }
    t = read_token();
  }

  std::vector<token_t> body;
  while(t.type != TOK_TYPE_NEWLINE && t.type != TOK_TYPE_END) {
    body.push_back(t);
    t = read_token();
  }
  if(funclike)
    add_macro_funclike(macro_name, args_name, body);
  else
    add_macro_object(macro_name, body);
}

void Lexer::read_undef() {
  std::string macro;
  auto t = read_token();
  if(t.type == TOK_TYPE_IDENT)
    macro = t.val;
  auto it = macro_map.find(macro);
  if(it != macro_map.end()) // if macro was declared
    macro_map.erase(it);
}

token_t Lexer::read_defined_op() {
  std::string macro_name;
  auto t = read_token();
  if(t.val == ("(")) {
    macro_name = read_token().val;
    if(read_token().val != ")") error("error(%d): expected ')'", t.line);
  } else {
    macro_name = t.val;
  }
  // std::cout << "MACRO " << macro_name << "DEFINED? " << macro_map.count(macro_name) << std::endl;;
  return is_defined(macro_name) ?
    token_t(TOK_TYPE_NUMBER, "1", t.line) :
    token_t(TOK_TYPE_NUMBER, "0", t.line);
}

Token Lexer::read_expr_line() {
  Token expr_line;
  for(;;) {
    auto t = read_token();
    if(t.type == TOK_TYPE_NEWLINE || t.type == TOK_TYPE_END) break;
    if(t.val == ("defined")) {
      expr_line.token.push_back(read_defined_op());
    } else {
      auto tok = t;
      if(is_defined(tok.val)) {
        replace_macro(tok.val);
      } else if(tok.type == TOK_TYPE_IDENT) {
        // INQCC: identifiers are replaced by 0
        expr_line.token.push_back(token_t(TOK_TYPE_NUMBER, "0"));
      } else {
        expr_line.token.push_back(tok);
      }
    }
  }
  return expr_line;
}

bool Lexer::read_constexpr() {
  auto tok = read_expr_line();
  tok.add_symbol_tok(";", 0);
  tok.add_end_tok();
  puts("EVAL");
  // tok.show();
  tok.seek(0);
  Parser parser;
  auto expr = parser.run(tok, true)[0];
  bool cond = eval_constexpr(expr);
  return cond;
}

void Lexer::do_read_if(bool m) {
  cond_stack.push(m);
  if(!m) skip_cond_include();
}

void Lexer::read_if() {
  do_read_if(read_constexpr());
}

void Lexer::read_elif() {
  if(cond_stack.top() || !read_constexpr())
    skip_cond_include();
  else
    cond_stack.top() = true;
}

void Lexer::read_ifdef() {
  auto t = read_token();
  if(t.type != TOK_TYPE_IDENT) error("error: in pp");
  std::string macro = t.val;
  do_read_if( is_defined(macro) );
}

void Lexer::read_ifndef() {
  auto t = read_token();
  if(t.type != TOK_TYPE_IDENT) error("error: in pp");
  std::string macro = t.val;
  do_read_if( !is_defined(macro) );
}

void Lexer::read_else() {
  if(cond_stack.empty()) error("error: in pp");
  if(cond_stack.top()) // if immediately before cond is true
    skip_cond_include();
}

void Lexer::skip_cond_include() {
  int nest = 0;
  for(;;) {
    auto t = read_token();
    if(t.val != "#") { continue; }
    t = read_token();
    if(!nest && (t.val == "else" || t.val == ("elif") || t.val == ("endif"))) {
      buffer.push_back(token_t(TOK_TYPE_SYMBOL, "#"));
      buffer.push_back(t);
      return;
    }
    if(t.val == ("if") || t.val == ("ifdef") || t.val == ("ifndef"))
      nest++;
    else if(nest && t.val == ("endif"))
      nest--;
  }
}

bool Lexer::is_defined(std::string name) {
  // std::cout << name << " " << macro_map.count(name) << std::endl;
  return macro_map.count(name);
}

void Lexer::replace_macro(std::string macro_name) {
  auto macro = macro_map[macro_name];
  puts("macro-entry");
  // std::cout << macro.name << std::endl;
  // macro.rep.show();
  puts("END");
  switch(macro.type) {
    case DEFINE_MACRO:          replace_macro_object(macro);
                                break;
    case DEFINE_FUNCLIKE_MACRO: replace_macro_funclike(macro);
                                break;
  }
}

std::string Lexer::stringize(std::vector<token_t> &tok) {
  std::string str;
  for(auto s : tok)
    str += (s.space && !str.empty() ? " " : "") +
      (s.type == TOK_TYPE_STRING ?
       "\"" + s.val + "\"" :
       s.type == TOK_TYPE_CHAR   ?
       "\'" + s.val + "\'" : s.val);
  return str;
}

void Lexer::subst_macro(Token &tok, Token args, macro_t macro) {
  auto body = macro.rep;
  // for(int i = 0; i < macro.rep.token.size(); i++) {
  // }
}

void Lexer::replace_macro_object(macro_t macro) {
  auto rep_tok = macro.rep;
  puts("macro-object");
  // rep_tok.show();
  auto push_to_buffer = [&](token_t t) {
    t.hideset[macro.name] = true;
    buffer.push_back(t);
  };
  while(!rep_tok.is_end()) {
    push_to_buffer(rep_tok.get());
    rep_tok.next();
  }
}

void Lexer::replace_macro_funclike(macro_t macro) {
  std::vector< std::vector<token_t> > args;
  int nest = 1;
  // funclike macro
  auto t = read_token();
  if(t.val != "(") error("error(%d): expected '('", t.line); //token.expect_skip("(");
  // TODO: this method should be a function
  t = read_token();
  // puts("BEGIN_ARG");
  while(1) {
    std::vector<token_t> arg;
    while(1) {
      if(t.val == "(") nest++; if(t.val == ")") nest--;
      if((t.val == ")" && !nest) ||
         (nest == 1 && t.val == ",")) break;
      arg.push_back(t);
      t = read_token();
    }
    args.push_back(arg);
    if(t.val == ")" && !nest) break;
    if(t.val != ",") error("error(%d): expected ','", t.line);
    t = read_token();
  }
  // puts("END_ARG");

  auto &tok = macro.rep;

  // TODO: implement subst and remove this redundancy code!
  auto push_to_buffer = [&](token_t t) {
    t.hideset[macro.name] = true;
    buffer.push_back(t);
  };
  for(; !tok.is_end();) {
    bool expand = false;
    bool stringify = tok.skip("#");
    bool cat = stringify && tok.skip("#");
    if(cat) stringify = false;
    for(size_t i = 0; i < macro.args.size(); i++) {
      if(tok.skip(macro.args[i])) { //it->token.val == macro.args[i]) {
        if(stringify) {
          std::string str = stringize(args[i]);
          push_to_buffer(token_t(TOK_TYPE_STRING, str));
        } else {
          for(auto a : args[i]) {
            if(!cat && !a.hideset.count(macro.name) && is_defined(a.val)) // TODO: WTF?!
              replace_macro(a.val);
            else
              push_to_buffer(a);
          }
        }
        expand = true;
        break;
      }
    }
    if(!expand) {
      push_to_buffer(tok.next());
    }
    if(cat) {
      auto b = buffer.back().val;
      buffer.pop_back();
      if(buffer.empty()) {
        push_to_buffer(token_t(TOK_TYPE_IDENT, b));
      } else
        buffer.back().val += b;
    }
  }
}

void Lexer::add_macro_object(std::string name, std::vector<token_t> rep) {
  macro_t def;
  def.name = name;
  def.type = DEFINE_MACRO;
  def.rep = Token(rep);
  macro_map[name] = def;
}

void Lexer::add_macro_funclike(std::string name, std::vector<std::string> args_name, std::vector<token_t> rep) {
  macro_t def;
  def.name = name;
  def.type = DEFINE_FUNCLIKE_MACRO;
  def.args = args_name;
  def.rep = Token(rep);
  macro_map[name] = def;
}
