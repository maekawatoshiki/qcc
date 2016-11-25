#include "pp.hpp"
#include "lexer.hpp"

Token Preprocessor::run(Token tok) {
  token = tok;
  while(token.get().type != TOK_TYPE_END) {
    if(token.skip("#")) {
      if(token.skip("include")) read_include();
      else if(token.skip("define"))  read_define();
      else if(token.skip("undef"))   read_undef();
      else if(define_map.count(token.get().val)) replace_macro();
    } else token.skip();
  }
  token.pos = 0;
  return token;
}

void Preprocessor::read_define() {
  int pos_bgn = token.pos - 2;
  auto it_bgn = token.token.begin() + token.pos - 2;
  std::string macro_name; bool funclike = false;
  std::vector<std::string> args_name;
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
  puts(macro_name.c_str());
  int cur_line = token.get().line;
  std::vector<token_t> content;
  while(cur_line==token.get().line)
    content.push_back(token.next());
  if(funclike) add_define_funclike_macro(macro_name, args_name, content);
  else add_define_macro(macro_name, content);
  auto it_end = token.token.begin() + token.pos;
  token.token.erase(it_bgn, it_end);
  token.pos = pos_bgn;
  // std::cout << token.get().val << std::endl;
}

void Preprocessor::read_undef() {
  int pos_bgn = token.pos;
  auto it_bgn = token.token.begin() + token.pos - /* '#' 'define' */2;
  std::string macro;
  if(token.get().type == TOK_TYPE_IDENT)
    macro = token.next().val;
  auto it = define_map.find(macro);
  if(it != define_map.end()) // if macro was declared
    define_map.erase(it);
  auto it_end = token.token.begin() + token.pos;
  token.token.erase(it_bgn, it_end);
  token.pos = pos_bgn;
}

void Preprocessor::read_include() {
  auto it_bgn = token.token.begin() + token.pos - 2;
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
  auto it_end = token.token.begin() + token.pos;
  token.token.erase(it_bgn, it_end);
  token.token.insert(it_bgn, include_tok.token.begin(), include_tok.token.end() /*TOK_TYPE_END=*/-1);
  // token.pos = include_tok.token.size() + pos_bgn;
}
void Preprocessor::replace_macro() {
  auto macro = define_map[token.get().val];
  if(macro.type == DEFINE_MACRO) {
    token.token.erase(token.token.begin() + token.pos);
    token.token.insert(token.token.begin() + token.pos, macro.rep.begin(), macro.rep.end());
    return; 
  }
  // funclike macro
  auto repd_bgn = token.pos;
  token.skip(); // IDENT
  token.expect_skip("(");
  std::vector< std::vector<token_t> > args;
  while(token.get().type != TOK_TYPE_END) {
    std::vector<token_t> arg;
    while(1) {
      arg.push_back(token.next());
      if(token.is(")") || token.is(",")) break;
    }
    args.push_back(arg);
    if(token.skip(")")) break;
    token.expect_skip(",");
  }
  auto repd_end = token.pos;
  std::vector<token_t> rep_to = macro.rep;
  for(int i = 0; i < macro.args.size(); i++) {
    auto it = std::find_if(rep_to.begin(), rep_to.end(), [&](token_t &t) {
        return t.val == macro.args[i]; });
    while(it != rep_to.end()) {
      rep_to.erase(it);
      rep_to.insert(it, args[i].begin(), args[i].end());
      it = std::find_if(rep_to.begin(), rep_to.end(), [&](token_t &t) {
          return t.val == macro.args[i]; });
    }
  }
  token.token.erase(token.token.begin() + repd_bgn, token.token.begin() + repd_end);
  token.token.insert(token.token.begin() + repd_bgn, rep_to.begin(), rep_to.end());
  token.pos = repd_bgn + rep_to.size();
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
