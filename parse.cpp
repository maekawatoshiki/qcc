#include "parse.hpp"

void show_ast(AST *ast) {
  switch(ast->get_type()) {
    case AST_FUNCTION_DEF: {
      FunctionDefAST *a = (FunctionDefAST *)ast;
      std::cout << "(def (" << a->ret_type->to_string() << ") " << a->name << " ";
      show_ast(a->body);
      std::cout << ")";
    } break;
    case AST_FUNCTION_PROTO: {
      FunctionProtoAST *a = (FunctionProtoAST *)ast;
      std::cout << "(proto (" << a->ret_type->to_string() << ") " << a->name << "(";
      for(auto t : a->args)
        std::cout << t->to_string() << " ";
      std::cout << ") ";
    } break;
    case AST_BLOCK: {
      BlockAST *a = (BlockAST *)ast;
      for(auto st : a->body)
        show_ast(st);
    } break;
    case AST_FUNCTION_CALL: {
      FunctionCallAST *a = (FunctionCallAST *)ast;
      std::cout << "(call " << a->name << " ";
      for(auto st : a->args) {
        std::cout << "("; show_ast(st); std::cout << ") ";
      }
      std::cout << ") ";
    } break;
    case AST_IF: {
      IfAST *a = (IfAST *)ast;
      std::cout << "(if ";
      show_ast(a->cond); std::cout << " (";
      show_ast(a->b_then); std::cout << ")";
      if(a->b_else) {
        std::cout << " (";
        show_ast(a->b_else); std::cout << ")";
      }
      std::cout << ")";
    } break;
    case AST_WHILE: {
      WhileAST *a = (WhileAST *)ast;
      std::cout << "(while ";
      show_ast(a->cond); std::cout << " ";
      show_ast(a->body);
      std::cout << ")";
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
    case AST_INDEX: {
      IndexAST *a = (IndexAST *)ast;
      std::cout << "([] ";
      show_ast(a->ary);
      std::cout << " ";
      show_ast(a->idx);
      std::cout << ") ";
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
      if(a->expr) show_ast(a->expr);
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
  op_prec["%"] =  400;  
  auto a = eval();
  for(auto b : a)
    show_ast(b);
  puts("");
  return a;
}

AST_vec Parser::eval() {
  AST_vec program;
  while(token.get().type != TOK_TYPE_END) {
    program.push_back(statement());
    while(token.skip(";"));
  }
  return program;
}

AST *Parser::statement() {
  if(is_function_def()) return make_function();
  if(token.is("if")) return make_if();
  if(token.is("while")) return make_while();
  if(token.is("return")) return make_return();
  if(token.is("{")) return make_block();
  if(is_function_proto()) return make_function_proto();
  if(is_var_declaration()) return make_var_declaration();
  return expr_entry();
}

AST *Parser::make_function() {
  // puts("MAKE_FUNCTION");
  Type *ret_type = skip_declarator();
  std::string name = token.next().val;
  if(token.skip("(")) {
    std::vector<argument_t *> args;
    while(!token.skip(")")) {
      Type *type = skip_declarator();
      std::string name = token.next().val;
      std::vector<int> ary = skip_array();
      for(auto e : ary)
        type = new Type(TY_ARRAY, e, type);
      args.push_back(new argument_t(type, name));
      token.skip(",");
    }
    AST *body;
    body = statement();
    return new FunctionDefAST(name, ret_type, args, body);
  } else puts("err");
  return nullptr;
}

AST *Parser::make_function_proto() {
  // puts("MAKE_PROTO");
  Type *ret_type = skip_declarator();
  std::string name = token.next().val;
  if(token.skip("(")) {
    Type_vec args_type;
    while(!token.skip(")")) {
      Type *type = skip_declarator();
      if(token.get().type == TOK_TYPE_IDENT) token.skip();
      std::vector<int> ary = skip_array();
      for(auto e : ary)
        type = new Type(TY_ARRAY, e, type);
      args_type.push_back(type);
      token.skip(",");
    }
    return new FunctionProtoAST(name, ret_type, args_type);
  } else puts("err");
  return nullptr;
}

AST *Parser::make_block() { 
  AST_vec body;
  if(token.skip("{")) {
    while(!token.skip("}")) {
      auto st = statement();
      while(token.skip(";"));
      if(st) body.push_back(st);
    }
    return new BlockAST(body);
  }
  return nullptr;
}

AST *Parser::make_if() {
  if(token.skip("if")) {
    token.skip("(");
    AST *cond = expr_entry();
    token.skip(")");
    AST *b_then = statement();
    if(token.skip("else")) {
      puts("else");
      AST *b_else = statement();
      return new IfAST(cond, b_then, b_else);
    } else return new IfAST(cond, b_then);
  }
  return nullptr;
}

AST *Parser::make_while() {
  if(token.skip("while")) {
    token.skip("(");
    AST *cond = expr_entry();
    token.skip(")");
    AST *body = statement();
    return new WhileAST(cond, body);
  }
  return nullptr;
}

AST *Parser::make_return() {
  if(token.skip("return")) {
    if(token.skip(";"))
      return new ReturnAST(nullptr);
    ReturnAST *ret = new ReturnAST(expr_entry());
    token.skip(";");
    return ret;
  }
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
    std::vector<int> ary = skip_array();
    for(auto e : ary)
      type = new Type(TY_ARRAY, e, type);
    decls.push_back(new declarator_t(type, name));
    token.skip(",");
  }
  return new VarDeclarationAST(decls);
}

bool Parser::is_function_proto() {
  int pos = token.pos;
  if(skip_declarator()) {
    token.skip(); // function name
    if(token.skip("(")) {
      while(!token.skip(")")) token.skip();
      if(token.skip(";")) {
        token.pos = pos;
        return true;
      }
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

std::vector<int> Parser::skip_array() {
  std::vector<int> ary;
  while(token.skip("[")) {
    int ary_size = -1;
    if(token.get().type == TOK_TYPE_NUMBER) 
      ary_size = atoi(token.next().val.c_str());
    token.skip("]");
    ary.push_back(ary_size);
  }
  return ary;
}
