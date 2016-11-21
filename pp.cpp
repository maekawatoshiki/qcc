#include "pp.hpp"
#include "lexer.hpp"

Token Preprocessor::run(Token tok) {
  token = tok;
  while(token.get().type != TOK_TYPE_END) {
    if(token.skip("#")) {
      if(token.skip("include")) read_include();
      else if(token.skip("define")) read_define();
    } else token.skip();
  }
  // replace define
  for(auto t = token.token.begin(); t != token.token.end(); ++t) {
    if(define_map.count(t->val)) {
      auto macro = define_map[t->val];
      if(macro.type == DEFINE_MACRO) {
        token.token.erase(t);
        token.token.insert(t, macro.rep.begin(), macro.rep.end());
      } else {
        auto repd_bgn = t;
        ++t; // ident
        ++t; // (
        std::vector< std::vector<token_t> > args;
        while(1) {
          std::vector<token_t> arg;
          while(1) {
            arg.push_back(*(t++));
            if(t->val == ")") break;
            if(t->val == ",") break;
          }
          args.push_back(arg);
          if(t->val == ")") break;
          if(t->val == ",") ++t;
        }
        ++t; // )
        auto repd_end = t;
        std::vector<token_t> rep_to = macro.rep;
        for(int i = 0; i < macro.args.size(); i++) {
          for(auto r = rep_to.begin(); r != rep_to.end(); ++r) {
            if(r->val == macro.args[i]) {
              rep_to.erase(r);
              rep_to.insert(r, args[i].begin(), args[i].end());
            }
          }
        }
        token.token.erase(repd_bgn, repd_end);
        token.token.insert(repd_bgn, rep_to.begin(), rep_to.end());
        t = repd_bgn + rep_to.size();
      }
    }
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

void Preprocessor::read_include() {
  // int pos_bgn = token.pos - 2;
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
