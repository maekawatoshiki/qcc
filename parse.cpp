#include "parse.hpp"
#include "codegen.hpp"

void show_ast(AST *ast) {
  // switch(ast->get_type()) {
  //   case AST_FUNCTION_DEF: {
  //     FunctionDefAST *a = (FunctionDefAST *)ast;
  //     std::cout << "(def ("; a->ret_type->dump(); std::cout << ") " << a->name << " ";
  //     for(auto st : a->body)
  //       show_ast(st);
  //     std::cout << ")";
  //   } break;
  //   case AST_FUNCTION_PROTO: {
  //     FunctionProtoAST *a = (FunctionProtoAST *)ast;
  //     std::cout << "(proto ("; a->ret_type->dump(); std::cout << ") " << a->name << "(";
  //     for(auto t : a->args)
  //       t->dump(); std::cout << " ";
  //     std::cout << ") ";
  //   } break;
  //   case AST_BLOCK: {
  //     BlockAST *a = (BlockAST *)ast;
  //     for(auto st : a->body)
  //       show_ast(st);
  //   } break;
  //   case AST_FUNCTION_CALL: {
  //     FunctionCallAST *a = (FunctionCallAST *)ast;
  //     std::cout << "(call " << a->name << " ";
  //     for(auto st : a->args) {
  //       std::cout << "("; show_ast(st); std::cout << ") ";
  //     }
  //     std::cout << ") ";
  //   } break;
  //   case AST_IF: {
  //     IfAST *a = (IfAST *)ast;
  //     std::cout << "(if ";
  //     show_ast(a->cond); std::cout << " (";
  //     show_ast(a->b_then); std::cout << ")";
  //     if(a->b_else) {
  //       std::cout << " (";
  //       show_ast(a->b_else); std::cout << ")";
  //     }
  //     std::cout << ")";
  //   } break;
  //   case AST_WHILE: {
  //     WhileAST *a = (WhileAST *)ast;
  //     std::cout << "(while ";
  //     show_ast(a->cond); std::cout << " ";
  //     show_ast(a->body);
  //     std::cout << ")";
  //   } break;
  //   case AST_UNARY: {
  //     UnaryAST *a = (UnaryAST *)ast;
  //     std::cout << "(" << a->op << " ";
  //     show_ast(a->expr);
  //     std::cout << ") ";
  //   } break;
  //   case AST_BINARY: {
  //     BinaryAST *a = (BinaryAST *)ast;
  //     std::cout << "(" << a->op << " ";
  //     show_ast(a->lhs);
  //     std::cout << " ";
  //     show_ast(a->rhs);
  //     std::cout << ") ";
  //   } break;
  //   case AST_DOT: {
  //     DotOpAST *a = (DotOpAST *)ast;
  //     std::cout << "(. ";
  //     show_ast(a->lhs);
  //     std::cout << " ";
  //     show_ast(a->rhs);
  //     std::cout << ") ";
  //   } break;
  //   case AST_VAR_DECLARATION: {
  //     VarDeclarationAST *a = (VarDeclarationAST *)ast;
  //     std::cout << "(var-decl ";
  //     for(auto decl : a->decls) {
  //       std::cout << "("; decl->type->dump(); std::cout << ", " << decl->name << ") "; }
  //     std::cout << ") ";
  //   } break;
  //   case AST_VARIABLE: {
  //     VariableAST *a = (VariableAST *)ast;
  //     std::cout << "(var " << a->name << ") ";
  //   } break;
  //   case AST_INDEX: {
  //     IndexAST *a = (IndexAST *)ast;
  //     std::cout << "([] ";
  //     show_ast(a->ary);
  //     std::cout << " ";
  //     show_ast(a->idx);
  //     std::cout << ") ";
  //   } break;
  //   case AST_ASGMT: {
  //     AsgmtAST *a = (AsgmtAST *)ast;
  //     std::cout << "(= (";
  //     show_ast(a->dst);
  //     std::cout << ") (";
  //     show_ast(a->src);
  //     std::cout << ")) ";
  //   } break;
  //   case AST_RETURN: {
  //     ReturnAST *a = (ReturnAST *)ast;
  //     std::cout << "(return ";
  //     if(a->expr) show_ast(a->expr);
  //     std::cout << ") ";
  //   } break;
  //   case AST_NUMBER: {
  //     NumberAST *a = (NumberAST *)ast;
  //     std::cout << a->number << " ";
  //   } break;
  //   case AST_STRING: {
  //     StringAST *a = (StringAST *)ast;
  //     std::cout << "\"" << a->str << "\" ";
  //   } break;
  // }
}

AST_vec Parser::run(Token tok) {
  token = tok;
  op_prec["=="] = 200;
  op_prec["!="] = 200;
  op_prec["<="] = 200;
  op_prec[">="] = 200;
  op_prec["<"] =  200;
  op_prec[">"] =  200;
  op_prec["&&"] =  150;
  op_prec["||"] =  150;
  op_prec["&"] =  150;
  op_prec["|"] =  150;
  op_prec["^"] =  150;
  op_prec["+"] =  300;
  op_prec["-"] =  300;
  op_prec["?"] =  300;
  op_prec["*"] =  400;
  op_prec["/"] =  400;
  op_prec["%"] =  400;  
  op_prec["."] =  500;  
  auto a = eval();
  for(auto b : a)
    show_ast(b);
  puts("");
  return a;
}

AST_vec Parser::eval() {
  AST_vec program;
  while(token.get().type != TOK_TYPE_END) {
    auto st = statement_top();
    if(st) program.push_back(st);
    while(token.skip(";"));
  }
  return program;
}

AST *Parser::statement_top() {
  if(is_function_def()) return make_function();
  if(is_function_proto()) return make_function_proto();
  if(token.skip("typedef"))  read_typedef();
  if(is_type()) return read_declaration();
  return nullptr;
}

AST *Parser::statement() {
  if(token.skip(";")) return statement();
  if(is_type()) return read_declaration();
  if(token.is("if")) return make_if();
  if(token.is("while")) return make_while();
  if(token.is("for")) return make_for();
  if(token.is("return")) return make_return();
  if(token.skip("break")) return make_break();
  if(token.skip("continue")) return make_continue();
  if(token.is("{")) return make_block();
  auto e = expr_entry();
  token.skip(";");
  return e;
}

AST *Parser::read_declaration() {
  llvm::Type *basety = skip_type_spec();
  if(token.skip(";")) return nullptr;
  std::vector<declarator_t *> decls;
  while(1) {
    std::string name;
    llvm::Type *type = read_declarator(name, basety);
    AST *init_expr = nullptr;
    if(token.skip("=")) 
      init_expr = expr_entry();
    decls.push_back(new declarator_t(type, name, init_expr));
    if(token.skip(";")) break;
    token.expect_skip(",");
  }
  return new VarDeclarationAST(decls);
}

llvm::Type *Parser::read_declarator(std::string &name, llvm::Type *basety) {
  if(token.skip("*")) {
    return read_declarator(name, basety->getPointerTo());
  }
  if(token.get().type == TOK_TYPE_IDENT) {
    name = token.next().val;
    return read_declarator_tail(basety);
  }
  return read_declarator_tail(basety);
}

llvm::Type *Parser::read_declarator_tail(llvm::Type *basety) {
  if(token.skip("[")) {
    return read_declarator_array(basety);
  }
  return basety;
}

llvm::Type *Parser::read_declarator_array(llvm::Type *basety) {
  int len;
  if(token.skip("]")) {
    len = -1;
  } else {
    if(token.get().type != TOK_TYPE_NUMBER)
      error("error(%d): expected number literal", token.get().line);
    len = atoi(token.next().val.c_str());
    token.expect_skip("]");
  }
  llvm::Type *t = read_declarator_tail(basety);
  if(len == -1) return t->getPointerTo();
  return llvm::ArrayType::get(t, len);
}

AST *Parser::make_function() {
  // puts("MAKE_FUNCTION");
  llvm::Type *ret_type = read_type_declarator();
  std::string name = token.next().val;
  if(token.skip("(")) {
    std::vector<argument_t *> args;
    while(!token.skip(")")) {
      llvm::Type *type = read_type_declarator();
      std::string name = token.next().val;
      std::vector<int> ary = skip_array();
      std::reverse(ary.begin(), ary.end());
      for(auto e : ary) {
        if(e == -1)
          type = type->getPointerTo();
        else
          type = llvm::ArrayType::get(type, e); //llvm::Type(TY_ARRAY, e, type);
      }
      args.push_back(new argument_t(type, name));
      if(token.skip(")")) break;
      token.expect_skip(",");
    }
    AST_vec body;
    token.expect_skip("{");
    while(!token.skip("}")) {
      auto st = statement();
      if(st) body.push_back(st);
      while(token.skip(";"));
    }
    return new FunctionDefAST(name, ret_type, args, body);
  } else puts("err");
  return nullptr;
}

AST *Parser::make_function_proto() {
  // puts("MAKE_PROTO");
  llvm::Type *ret_type = read_type_declarator();
  std::string name = token.next().val;
  if(token.skip("(")) {
    Type_vec args_type;
    while(!token.skip(")")) {
      llvm::Type *type = read_type_declarator();
      if(token.get().type == TOK_TYPE_IDENT) token.skip();
      std::vector<int> ary = skip_array();
      std::reverse(ary.begin(), ary.end());
      for(auto e : ary) {
        if(e == -1)
          type = type->getPointerTo();
        else
          type = llvm::ArrayType::get(type, e); //llvm::Type(TY_ARRAY, e, type);
      }
      args_type.push_back(type);
      if(token.skip(")")) break;
      token.expect_skip(",");
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

AST *Parser::make_break() {
  return new BreakAST;
}
AST *Parser::make_continue() {
  return new ContinueAST;
}

AST *Parser::make_if() {
  if(token.skip("if")) {
    token.expect_skip("(");
    AST *cond = expr_entry();
    token.expect_skip(")");
    AST *b_then = statement();
    if(token.skip("else")) {
      AST *b_else = statement();
      return new IfAST(cond, b_then, b_else);
    } else return new IfAST(cond, b_then);
  }
  return nullptr;
}

AST *Parser::make_while() {
  if(token.skip("while")) {
    token.expect_skip("(");
    AST *cond = expr_entry();
    token.expect_skip(")");
    AST *body = statement();
    return new WhileAST(cond, body);
  }
  return nullptr;
}

AST *Parser::make_for() {
  if(token.skip("for")) {
    token.expect_skip("(");
    AST *init;
    if(is_type()) init = read_declaration();
    else init = expr_entry();
    token.skip(";");
    AST *cond = expr_entry();
    token.expect_skip(";");
    AST *reinit = expr_entry();
    token.expect_skip(")");
    AST *body = statement();
    return new ForAST(init, cond, reinit, body);
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

llvm::Type *Parser::make_struct_declaration() {
  if(!token.skip("struct")) return nullptr;
  std::string name;
  if(token.get().type == TOK_TYPE_IDENT) 
    name = token.next().val;
	if(name.empty()) [](std::string &str) {
		int len = 8; while(len--)
      str += (rand() % 26) + 65;
	}(name);
  BlockAST *decls = (BlockAST *)statement();
  llvm::StructType *new_struct = llvm::StructType::create(context, "struct." + name);
  std::vector<std::string> members_name;
  Type_vec field;
  for(auto a : decls->body) {
    if(a->get_type() == AST_VAR_DECLARATION) {
      VarDeclarationAST *decl = (VarDeclarationAST *)a;
      for(auto v : decl->decls)
        members_name.push_back(v->name);
    }
  }
  this->struct_list.add("struct." + name, members_name, new_struct);
  for(auto a : decls->body) {
    if(a->get_type() == AST_VAR_DECLARATION) {
      VarDeclarationAST *decl = (VarDeclarationAST *)a;
      for(auto v : decl->decls)
        field.push_back(v->type);
    }
  }
  if(new_struct->isOpaque())
    new_struct->setBody(field, false);
  return new_struct;
}

void Parser::read_typedef() {
  llvm::Type *from = skip_type_spec();
  if(token.get().type != TOK_TYPE_IDENT)
    error("error(%d): expected identifier", token.get().line);
  std::string name = token.next().val;
  typedef_map[name] = from;
  return;
}


bool Parser::is_function_proto() {
  int pos = token.pos;
  if(!read_type_declarator()) goto exit;
  token.skip(); // function name
  if(!token.skip("(")) goto exit;
  while(!token.skip(")")) token.skip();
  if(!token.skip(";")) goto exit;
  token.pos = pos;
  return true;
exit:
  token.pos = pos;
  return false;
}

bool Parser::is_function_def() {
  int pos = token.pos;
  if(!read_type_declarator()) goto exit;
  token.skip(); // function name
  if(!token.skip("(")) goto exit;
  while(token.get().val != ")" && token.get().type != TOK_TYPE_END) token.skip();
  if(!token.skip(")")) goto exit;
  if(!token.skip("{")) goto exit;
  token.pos = pos;
  return true;
exit:
  token.pos = pos;
  return false;
}

bool Parser::is_type() {
  auto cur = token.get().val;
  if(
      cur == "int"      ||
      cur == "char"     ||
      cur == "double"   ||
      cur == "struct"   ||
      cur == "union") {
    return true;
  } else if(typedef_map.count(cur)) {
    return true;
  }
  return false;
}

llvm::Type *Parser::read_type_declarator() {
  llvm::Type *type = skip_type_spec();
  if(type == nullptr) return nullptr;
  for(int i=skip_pointer(); i > 0; i--)
    type = type->getPointerTo(); //new llvm::Type(TY_PTR, type);
  return type;
}

llvm::Type *Parser::skip_type_spec() {
  bool is_struct = false;
  if((is_struct=token.is("struct")) || token.is("union")) {
    if(token.get(1).val == "{" || token.get(2).val == "{") // define struct or union
      return make_struct_declaration();
    token.skip();
    std::string name;
    if(token.get().type == TOK_TYPE_IDENT) 
      name = token.next().val; 
    else puts("err"); // TODO: add err check
    return this->struct_list.get("struct." + name)->llvm_struct;
  } else if(token.get().type == TOK_TYPE_IDENT) {
    std::string name = token.next().val;
    return to_llvm_type(name);
  } else if(token.skip("...")) {
    return nullptr; // null is variable argument
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
    token.expect_skip("]");
    ary.push_back(ary_size);
  }
  return ary;
}

llvm::Type *Parser::to_llvm_type(std::string ty) {
  if(ty == "int") {
    return builder.getInt32Ty();
  } else if(ty == "void") {
    return builder.getVoidTy();
  } else if(ty == "char") {
    return builder.getInt8Ty();
  } else if(ty == "double") {
    return builder.getDoubleTy();
  } else { // typedef
    return typedef_map[ty];
  }
}
