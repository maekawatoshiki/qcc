#include "parse.hpp"
#include "codegen.hpp"

AST_vec Parser::run(Token tok) {
  token = tok;
  op_prec["."] =  600;  
  op_prec["*"] =  500;
  op_prec["/"] =  500;
  op_prec["%"] =  500;  
  op_prec["+"] =  400;
  op_prec["-"] =  400;
  op_prec["=="] = 300;
  op_prec["!="] = 300;
  op_prec["<="] = 300;
  op_prec[">="] = 300;
  op_prec["<"] =  300;
  op_prec[">"] =  300;
  op_prec["&"] =  300;
  op_prec["|"] =  200;
  op_prec["^"] =  200;
  op_prec["&&"] = 200;
  op_prec["||"] = 200;
  op_prec["?"] =  100;
  return eval();
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
  if(token.skip("typedef")) { read_typedef(); return nullptr; }
  if(is_type()) return read_declaration();
  return nullptr;
}

AST *Parser::statement() {
  if(token.skip(";")) return statement();
  if(is_type()) return read_declaration();
  if(token.skip("if")) return make_if();
  if(token.skip("while")) return make_while();
  if(token.skip("for")) return make_for();
  if(token.skip("return")) return make_return();
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

llvm::Type *Parser::read_func_param(std::string &name) {
  llvm::Type *basety = builder.getInt32Ty();
  if(is_type()) basety = skip_type_spec();
  else error("error(%d): expected type specify", token.get().line);
  if(basety == nullptr) return basety;
  llvm::Type *type = read_declarator(name, basety);
  if(type->isArrayTy()) type = type->getArrayElementType()->getPointerTo();
  return type;
}

std::vector<argument_t *> Parser::read_declarator_param() {
  std::vector<argument_t *> args;

  if(token.is("void") && token.get(1).val == ")") { // e.g. int main(void)
    token.expect_skip("void");
    token.expect_skip(")");
    return args;
  } else if(token.skip(")")) return args; // e.g. int main()

  for(;;) {
    std::string name;
    llvm::Type *type = read_func_param(name);
    args.push_back(new argument_t(type, name));
    if(token.skip(")")) return args;
    token.expect_skip(",");
  }
  return args;
}

AST *Parser::make_function() {
  llvm::Type *ret_type = read_type_declarator();
  std::string name = token.next().val;
  token.expect_skip("(");
  auto args = read_declarator_param();
  AST_vec body;
  token.expect_skip("{");
  while(!token.skip("}")) {
    auto st = statement();
    if(st) body.push_back(st);
    while(token.skip(";"));
  }
  return new FunctionDefAST(name, ret_type, args, body);
}

AST *Parser::make_function_proto() {
  llvm::Type *ret_type = read_type_declarator();
  std::string name = token.next().val;
  token.expect_skip("(");
  auto args = read_declarator_param();
  return new FunctionProtoAST(name, ret_type, args);
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
  token.expect_skip("(");
  AST *cond = expr_entry();
  token.expect_skip(")");
  AST *b_then = statement();
  if(token.skip("else")) {
    AST *b_else = statement();
    return new IfAST(cond, b_then, b_else);
  } else return new IfAST(cond, b_then);
}

AST *Parser::make_while() {
  token.expect_skip("(");
  AST *cond = expr_entry();
  token.expect_skip(")");
  AST *body = statement();
  return new WhileAST(cond, body);
}

AST *Parser::make_for() {
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

AST *Parser::make_return() {
  if(token.skip(";"))
    return new ReturnAST(nullptr);
  ReturnAST *ret = new ReturnAST(expr_entry());
  token.skip(";");
  return ret;
}

llvm::Type *Parser::make_struct_declaration() {
  if(!token.skip("struct")) return nullptr;
  std::string name;
  if(token.get().type == TOK_TYPE_IDENT) 
    name = token.next().val;
  if(name.empty()) [](std::string &str) { // if name is empty, generate random name
    int len = 8; while(len--)
      str += (rand() % 26) + 65;
  }(name);

  llvm::StructType *new_struct = nullptr;
  auto t_strct = this->struct_list.get("struct." + name);
  if(!t_strct) {
    new_struct = llvm::StructType::create(context, "struct." + name);
    this->struct_list.add("struct." + name, std::vector<std::string>(), new_struct);
    t_strct = this->struct_list.get("struct." + name);
  } else new_struct = t_strct->llvm_struct;
  // this if block should be function. not beautiful
  if(token.is("{")) {
    BlockAST *decls = (BlockAST *)statement();
    std::vector<std::string> members_name;
    Type_vec field;
    for(auto a : decls->body) {
      if(a->get_type() == AST_VAR_DECLARATION) {
        VarDeclarationAST *decl = (VarDeclarationAST *)a;
        for(auto v : decl->decls)
          members_name.push_back(v->name);
      } else error("error: struct fields must be varaible declaration");
    }
    t_strct = this->struct_list.get("struct." + name);
    t_strct->members_name = members_name;

    for(auto a : decls->body) {
      if(a->get_type() == AST_VAR_DECLARATION) {
        VarDeclarationAST *decl = (VarDeclarationAST *)a;
        for(auto v : decl->decls)
          field.push_back(v->type);
      }
    }
    if(new_struct->isOpaque())
      new_struct->setBody(field, false);
  } else if(token.is(";")) { // prototype (e.g. struct A;)
  }
  return new_struct;
}

llvm::Type *Parser::make_enum_declaration() {
  if(!token.skip("enum")) return nullptr;
  std::string name;
  if(token.get().type == TOK_TYPE_IDENT) 
    name = token.next().val;
  if(token.skip("{")) {
    std::vector<std::string> consts;
    while(!token.skip("}")) {
      if(token.get().type != TOK_TYPE_IDENT) error("ERR");
      consts.push_back(token.next().val);
      if(token.skip("}")) break;
      token.expect_skip(",");
    }
    for(int i = 0; i < consts.size(); i++) 
      enum_list[ consts[i] ] = new NumberAST(i);
  }
  return builder.getInt32Ty();
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
      cur == "void"     ||
      cur == "unsigned" ||
      cur == "signed"   ||
      cur == "int"      ||
      cur == "char"     ||
      cur == "short"    ||
      cur == "double"   ||
      cur == "struct"   ||
      cur == "enum"     ||
      cur == "union"    ||
      cur == "...") {
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
  if(token.is("struct") || token.is("union")) return read_struct_union_type();
  else if(token.is("enum")) return read_enum_type();
  else if(token.get().type == TOK_TYPE_IDENT) return read_primitive_type();
  else if(token.skip("...")) return nullptr; // null is variable argument
  return nullptr;
}

llvm::Type *Parser::read_struct_union_type() {
  bool is_struct = token.is("struct");
  if( token.get(1).val == "{" || // struct { ... }
      token.get(2).val == "{" || // struct NAME { ... }
      token.get(2).val == ";") {//   struct NAME; (prototype?)
    if(is_struct)
      return make_struct_declaration(); // TODO: union implement
  }
  token.skip();
  std::string name;
  if(token.get().type == TOK_TYPE_IDENT) 
    name = token.next().val; 
  else puts("err"); // TODO: add err check
  return this->struct_list.get("struct." + name)->llvm_struct;
}

llvm::Type *Parser::read_enum_type() {
  if( token.get(1).val == "{" || // struct { ... }
      token.get(2).val == "{") // struct NAME { ... }
    return make_enum_declaration();
  token.skip("enum");
  if(token.get().type == TOK_TYPE_IDENT)
    token.skip();
  return builder.getInt32Ty();
}

llvm::Type *Parser::read_primitive_type() {
  std::string name = token.next().val;
  if(name == "signed" || name == "unsigned") 
    name = token.next().val;
  return to_llvm_type(name);
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
  } else if(ty == "short") {
    return builder.getInt16Ty();
  } else if(ty == "void") {
    return builder.getVoidTy();
  } else if(ty == "char") {
    return builder.getInt8Ty();
  } else if(ty == "double") {
    return builder.getDoubleTy();
  } else { // typedef
    if(!typedef_map.count(ty)) return nullptr;
    return typedef_map[ty];
  }
}
