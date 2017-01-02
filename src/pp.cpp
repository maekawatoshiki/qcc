#include "pp.hpp"
#include "lexer.hpp"
#include "parse.hpp"

Token Preprocessor::run(Token tok) {
  token = tok;
  while(token.get().type != TOK_TYPE_END) {
    if(token.skip("#")) {
           if(token.skip("include"))             read_include();
      else if(token.skip("define"))              read_define();
      else if(token.skip("undef"))               read_undef();
      else if(token.skip("if"))                  read_if();
      else if(token.skip("ifdef"))               read_ifdef();
      else if(token.skip("ifndef"))              read_ifndef();
      else if(token.skip("else"))                read_else();
      else                                       skip_this_line();
    } else if(define_map.count(token.get().val)) replace_macro();
    else new_token.token.push_back(token.next());
  }
  token.pos = 0;
  return new_token;
}

void Preprocessor::read_define() {
  std::string macro_name; bool funclike = false;
  std::vector<std::string> args_name;
  int cur_line = token.get().line;
  if(token.get().type == TOK_TYPE_IDENT)
    macro_name = token.next().val;
  if(token.is("(") && !token.get().space) {
    token.skip();
    funclike = true;
    while(1) {
      args_name.push_back(token.next().val);
      if(token.skip(")")) break;
      token.expect_skip(",");
    }
  }
  std::vector<token_t> content;
  if(cur_line == token.get().line) {
    while(cur_line==token.get().line)
      content.push_back(token.next());
  }
  if(funclike) add_define_funclike_macro(macro_name, args_name, content);
  else add_define_macro(macro_name, content);
}

void Preprocessor::read_undef() {
  std::string macro;
  if(token.get().type == TOK_TYPE_IDENT)
    macro = token.next().val;
  auto it = define_map.find(macro);
  if(it != define_map.end()) // if macro was declared
    define_map.erase(it);
}

token_t Preprocessor::read_defined_op() {
  std::string macro_name;
  if(token.skip("(")) {
    macro_name = token.next().val;
    token.expect_skip(")");
  } else {
    macro_name = token.next().val;
  }
  std::cout << this->define_map.count(macro_name) << std::endl;;
  return this->define_map.count(macro_name) ? 
    token_t(TOK_TYPE_NUMBER, "1", token.get().line) : 
    token_t(TOK_TYPE_NUMBER, "0", token.get().line);
}

Token Preprocessor::read_expr_line() {
  Token expr_line;
  int cur_line = token.get().line;
  for(;;) {
    if(cur_line != token.get().line) break;
    if(token.skip("defined")) {
      expr_line.token.push_back(read_defined_op());
    } else {
      expr_line.token.push_back(token.next());
    }
  }
  return expr_line;
}

bool Preprocessor::read_constexpr() {
  // TODO: implement processing of const expression
  auto tok = read_expr_line();
  tok.add_symbol_tok(";", 0);
  tok.add_end_tok();
  tok.show();
  tok.pos = 0;
  Parser parser;
  auto expr = parser.run(tok, true)[0];
  bool cond = eval_constexpr(expr);
  return cond;
}

void Preprocessor::do_read_if(bool m) {
  cond_stack.push(m);
  if(!m) skip_cond_include();
}

void Preprocessor::read_if() {
  do_read_if(read_constexpr());
}

void Preprocessor::read_ifdef() {
  if(token.get().type != TOK_TYPE_IDENT) error("error: in pp");
  std::string macro = token.next().val;
  do_read_if( define_map.count(macro) );
}

void Preprocessor::read_ifndef() {
  if(token.get().type != TOK_TYPE_IDENT) error("error: in pp");
  std::string macro = token.next().val;
  do_read_if( !define_map.count(macro) );
}

void Preprocessor::read_else() {
  if(cond_stack.empty()) error("error: in pp");
  if(cond_stack.top()) // if immediately before cond is true
    skip_cond_include();
}

void Preprocessor::skip_cond_include() {
  int nest = 0;
  for(;;) {
    if(!token.skip("#")) { token.skip(); continue; }
    if(!nest && (token.skip("else") || token.skip("elif") || token.skip("endif")))
      return;
    if(token.skip("if") || token.skip("ifdef") || token.skip("ifndef"))
      nest++;
    else if(nest && token.skip("endif"))
      nest--;
  }
}

void Preprocessor::read_include() {
  std::string file_name;
  if(token.skip("<")) {
    while(!token.skip(">")) file_name += token.next().val;
  } else if(token.get().type == TOK_TYPE_STRING) {
    file_name = token.next().val;
  }
  std::string include_content = [&]() -> std::string {
    std::ifstream ifs_src(default_include_path + file_name);
    if(!ifs_src) { puts("file not found"); return ""; }
    std::istreambuf_iterator<char> it(ifs_src), last;
    std::string src_all(it, last);
    return src_all;
  }();
  Lexer lex; Token include_tok = lex.run(include_content);
  Preprocessor pp; include_tok = pp.run(include_tok);
  for(auto incl_def : pp.define_map) 
    define_map[incl_def.first] = incl_def.second;
  for(auto t : include_tok.token)
    new_token.token.push_back(t);
}
void Preprocessor::replace_macro() {
  auto macro = define_map[token.get().val];
  // normal macro
  if(macro.type == DEFINE_MACRO) {
    std::copy(macro.rep.begin(), macro.rep.end(), std::back_inserter(new_token.token));
    token.skip();
    return; 
  }

  // funclike macro
  token.skip(); // IDENT
  token.expect_skip("(");
  std::vector< std::vector<token_t> > args;
  while(token.get().type != TOK_TYPE_END) {
    std::vector<token_t> arg;
    int nest = 1;
    while(1) {
      if(token.is("(")) nest++; if(token.is(")")) nest--;
      if((token.is(")") && !nest) || token.is(",")) break;
      arg.push_back(token.next());
    }
    args.push_back(arg);
    if(token.skip(")") && !nest) break;
    token.expect_skip(",");
  }
  auto repd_end = token.pos;
  std::vector<token_t> rep_to = macro.rep;
  for(int i = 0; i < macro.args.size(); i++) {
    auto it = std::find_if(rep_to.begin(), rep_to.end(), [&](token_t &t) {
        return t.val == macro.args[i]; });
    while(it != rep_to.end()) {
      if((it-1)->val == "#") { 
        rep_to.erase(it-1); rep_to.erase(it);
        std::string str;
        for(auto s : args[i]) 
          str += s.val;
        auto new_str = token_t(TOK_TYPE_STRING, str); 
        rep_to.insert(it-1, new_str);
      } else {
        rep_to.erase(it);
        rep_to.insert(it, args[i].begin(), args[i].end());
      }
      it = std::find_if(rep_to.begin(), rep_to.end(), [&](token_t &t) {
          return t.val == macro.args[i]; });
    }
  }
  std::copy(rep_to.begin(), rep_to.end(), std::back_inserter(new_token.token));
}

void Preprocessor::skip_this_line() {
  int n = token.get().line;
  while(n == token.get().line) token.skip();
}

void Preprocessor::add_define_macro(std::string name, std::vector<token_t> rep) {
  define_t def;
  def.type = DEFINE_MACRO;
  def.rep = rep;
  define_map[name] = def;
}
void Preprocessor::add_define_funclike_macro(std::string name, std::vector<std::string> args_name, std::vector<token_t> rep) {
  define_t def;
  def.type = DEFINE_FUNCLIKE_MACRO;
  def.args = args_name;
  def.rep = rep;
  define_map[name] = def;
}
