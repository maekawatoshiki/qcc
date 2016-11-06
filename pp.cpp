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
      token.token.erase(t);
      token.token.insert(t, macro.begin(), macro.end());
    }
  }
  token.pos = 0;
  return token;
}

void Preprocessor::read_define() {
  int pos_bgn = token.pos - 2;
  auto it_bgn = token.token.begin() + token.pos - 2;
  std::string macro_name;
  if(token.get().type == TOK_TYPE_IDENT)
    macro_name = token.next().val;
  puts(macro_name.c_str());
  int cur_line = token.get().line;
  std::vector<token_t> content;
  while(cur_line==token.get().line)
    content.push_back(token.next());
  define_map[macro_name] = content;
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
