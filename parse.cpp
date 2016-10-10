#include "parse.hpp"

int Parser::run(Token tok) {
  token = tok;
  return eval();
}

int Parser::eval() {
  AST_vec program;
  // while(token.get().type != TOK_TYPE_END) {
  //   if(is_function_def()) program.push_back(make_function());
  //   token.skip();
  // }
  return 0;
}

AST *Parser::make_function() {
  return nullptr;
}

bool Parser::is_function_proto() {
  int pos = token.pos;
  if(skip_type()) {
    token.skip(); // function name
    if(token.get().type == TOK_TYPE_SYMBOL && 
        token.get().val == "(") { // this is function!
      token.pos = pos;
      return true;
    }
  }
  return false;
}

bool Parser::is_function_def() {
  int pos = token.pos;
  if(skip_type()) {
    token.skip(); // function name
    if(token.get().type == TOK_TYPE_SYMBOL && 
        token.get().val == "(") { 
      while(token.get().val != ")") token.skip();
      token.skip(")");
      if(token.skip("{")) {
        token.pos = pos;
        return true;
      }
    }
  }
  token.pos = pos;
  return false;
}

Type *Parser::skip_type() {
  if(token.get().val == "struct") {
    token.skip();
    std::string name;
    if(token.get().type == TOK_TYPE_IDENT) name=token.next().val; else puts("err"); // TODO: add err check
    while(token.get().type == TOK_TYPE_SYMBOL && token.get().val == "*")
      name += "*";
    return TypeTool::to_type(name);
  } else if(token.get().type == TOK_TYPE_IDENT) {
    std::string name = token.next().val;
    while(token.get().type == TOK_TYPE_SYMBOL && token.get().val == "*") {
      name += "*"; token.skip();
    }
    std::cout << "type name " << name << std::endl;
    return TypeTool::to_type(name);
  }
  return nullptr;
}

int Parser::skip_asterisk() {
  int count = 0;
  while(token.get().type == TOK_TYPE_SYMBOL && token.get().val == "*")
    count++;
  return count;
}
