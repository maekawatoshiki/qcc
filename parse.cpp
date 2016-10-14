#include "parse.hpp"

void show_ast(AST *ast) {
  switch(ast->get_type()) {
    case AST_FUNCTION_DEF: {
      FunctionDefAST *a = (FunctionDefAST *)ast;
      std::cout << "(def (" << a->ret_type->to_string() << ") " << a->name << " ";
      for(auto st : a->body) 
        show_ast(st);
      std::cout << ")";
    } break;
    case AST_FUNCTION_PROTO: {
      FunctionProtoAST *a = (FunctionProtoAST *)ast;
      std::cout << "(proto (" << a->ret_type->to_string() << ") " << a->name << "(";
      for(auto t : a->args)
        std::cout << t->to_string() << " ";
      std::cout << ") ";
    } break;
    case AST_FUNCTION_CALL: {
      FunctionCallAST *a = (FunctionCallAST *)ast;
      std::cout << "(call " << a->name << " ";
      for(auto st : a->args) {
        std::cout << "("; show_ast(st); std::cout << ") ";
      }
      std::cout << ") ";
    } break;
    case AST_RETURN: {
      ReturnAST *a = (ReturnAST *)ast;
      std::cout << "(return ";
      show_ast(a->expr);
      std::cout << ") ";
    } break;
    case AST_NUMBER: {
      NumberAST *a = (NumberAST *)ast;
      std::cout << a->number << " ";
    } break;
    case AST_STRING: {
      StringAST *a = (StringAST *)ast;
      std::cout << "\"" << a->str << "\" ";
    } break;
  }
}

int Parser::run(Token tok) {
  token = tok;
  auto a = eval();
  for(auto b : a)
    show_ast(b);
  puts("");
  return 0;
}

AST_vec Parser::eval() {
  AST_vec program;
  while(token.get().type != TOK_TYPE_END && !token.skip("}")) {
    program.push_back(statement());
    while(token.skip(";"));
  }
  return program;
}

AST *Parser::statement() {
  if(is_function_def()) return make_function();
  else if(is_function_proto()) return make_function_proto();
  else if(token.is("return")) return make_return();
  else return expr_entry();
  return nullptr;
}

AST *Parser::make_function() {
  // puts("FUNC");
  Type *ret_type = skip_type();
  std::string name = token.next().val;
  if(token.skip("(")) {
    std::vector<argument_t *> args;
    while(!token.skip(")")) {
      Type *type = skip_type();
      std::string name = token.next().val;
      args.push_back(new argument_t(type, name));
      token.skip(",");
    }
    token.skip("{");
    AST_vec body;
    body = eval();
    return new FunctionDefAST(name, ret_type, args, body);
  } else puts("err");
  return nullptr;
}

AST *Parser::make_function_proto() {
  // puts("PROTO");
  Type *ret_type = skip_type();
  std::string name = token.next().val;
  if(token.skip("(")) {
    Type_vec args_type;
    while(!token.skip(")")) {
      Type *type = skip_type();
      if(token.get().type == TOK_TYPE_IDENT) token.skip();
      args_type.push_back(type);
    }
    return new FunctionProtoAST(name, ret_type, args_type);
  } else puts("err");
  return nullptr;
}

AST *Parser::make_return() {
  // puts("RETURN");
  if(token.skip("return")) 
    return new ReturnAST(expr_entry()); 
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
  token.pos = pos;
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
    // std::cout << "type name " << name << std::endl;
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


