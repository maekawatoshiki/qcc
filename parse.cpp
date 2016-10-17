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
    case AST_BINARY: {
      BinaryAST *a = (BinaryAST *)ast;
      std::cout << "(" << a->op << " ";
      show_ast(a->lhs);
      std::cout << " ";
      show_ast(a->rhs);
      std::cout << ") ";
    } break;
    case AST_VAR_DECLARATION: {
      VarDeclarationAST *a = (VarDeclarationAST *)ast;
      std::cout << "(var-decl ";
      for(auto decl : a->decls) 
        std::cout << "(" << decl->type->to_string() << ", " << decl->name << ") ";
      std::cout << ") ";
    } break;
    case AST_VARIABLE: {
      VariableAST *a = (VariableAST *)ast;
      std::cout << "(var " << a->name << ") ";
    } break;
    case AST_ASGMT: {
      AsgmtAST *a = (AsgmtAST *)ast;
      std::cout << "(= (";
      show_ast(a->dst);
      std::cout << ") (";
      show_ast(a->src);
      std::cout << ")) ";
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

AST_vec Parser::run(Token tok) {
  token = tok;
  op_prec["="] =  100;
  op_prec["+="] = 100;
  op_prec["-="] = 100;
  op_prec["*="] = 100;
  op_prec["/="] = 100;
  op_prec["%="] = 100;
  op_prec["^="] = 100;
  op_prec["|="] = 100;
  op_prec["&="] = 100;
  op_prec["=="] = 200;
  op_prec["!="] = 200;
  op_prec["<="] = 200;
  op_prec[">="] = 200;
  op_prec["<"] =  200;
  op_prec[">"] =  200;
  op_prec["&"] =  150;
  op_prec["|"] =  150;
  op_prec["^"] =  150;
  op_prec["+"] =  300;
  op_prec["-"] =  300;
  op_prec["?"] =  300;
  op_prec["*"] =  400;
  op_prec["/"] =  400;
  op_prec["%"] = 400;
  auto a = eval();
  for(auto b : a)
    show_ast(b);
  puts("");
  return a;
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
  else if(is_var_declaration()) return make_var_declaration();
  else if(token.is("return")) return make_return();
  else return expr_entry();
  return nullptr;
}

AST *Parser::make_function() {
  // puts("FUNC");
  Type *ret_type = skip_declarator();
  std::string name = token.next().val;
  if(token.skip("(")) {
    std::vector<argument_t *> args;
    while(!token.skip(")")) {
      Type *type = skip_declarator();
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
  Type *ret_type = skip_declarator();
  std::string name = token.next().val;
  if(token.skip("(")) {
    Type_vec args_type;
    while(!token.skip(")")) {
      Type *type = skip_declarator();
      if(token.get().type == TOK_TYPE_IDENT) token.skip();
      args_type.push_back(type);
      token.skip(",");
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

AST *Parser::make_var_declaration() {
  Type *base_type = skip_type_spec();
  std::vector<declarator_t *> decls;
  while(!token.skip(";")) {
    Type *type = base_type;
    for(int i = skip_pointer(); i > 0; i--)
      type = new Type(TY_PTR, type);
    std::string name = token.next().val;
    decls.push_back(new declarator_t(type, name));
    token.skip(",");
  }
  return new VarDeclarationAST(decls);
}

bool Parser::is_function_proto() {
  int pos = token.pos;
  if(skip_declarator()) {
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
  if(skip_declarator()) {
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

bool Parser::is_var_declaration() {
  int pos = token.pos;
  if(skip_type_spec()) {
    skip_pointer();
    if(token.get().type == TOK_TYPE_IDENT) {
      token.pos = pos;
      return true;
    }
  }
  token.pos = pos;
  return false;
}

Type *Parser::skip_declarator() {
  Type *type = skip_type_spec();
  for(int i=skip_pointer(); i > 0; i--)
    type = new Type(TY_PTR, type);
  return type;
}

Type *Parser::skip_type_spec() {
  if(token.get().val == "struct" || token.get().val == "union") {
    token.skip();
    if(token.skip("{")) {
      while(!token.skip("}"));
      // TODO: implement
    } else {
      std::string name;
      if(token.get().type == TOK_TYPE_IDENT) name=token.next().val; else puts("err"); // TODO: add err check
      if(token.skip("{")) {
        while(!token.skip("}"));
        // TODO: implement
      } else 
        return TypeTool::to_type(name);
    }
  } else if(token.get().type == TOK_TYPE_IDENT) {
    std::string name = token.next().val;
    // std::cout << "type name " << name << std::endl;
    return TypeTool::to_type(name);
  } else if(token.skip("...")) {
    return new Type(TY_VARARG);
  }
  return nullptr;
}

int Parser::skip_pointer() {
  int count = 0;
  while(token.get().type == TOK_TYPE_SYMBOL && token.get().val == "*")
    count++, token.skip();
  return count;
}


