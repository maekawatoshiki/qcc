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
  int stg = STG_NONE;
  llvm::Type *basety = read_type_spec(stg);
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
  return new VarDeclarationAST(decls, stg);
}

llvm::Type *Parser::read_declarator(std::string &name, llvm::Type *basety) {
  std::vector<argument_t *> param;
  return read_declarator(name, basety, param);
}
llvm::Type *Parser::read_declarator(std::string &name, llvm::Type *basety, std::vector<argument_t *> &param) {
  if(token.skip("(")) { // TODO: WHAT A F**K CODE??!! FIXME!!
    int pos_bgn = token.pos;
    llvm::Type *s = builder.getVoidTy();
    llvm::Type *type = read_declarator(name, s);  
    token.expect_skip(")");
    auto ft = read_declarator_tail(basety, param);
    int pos_end = token.pos;
    token.pos = pos_bgn;
    type = read_declarator(name, ft);
    token.expect_skip(")");
    token.pos = pos_end;
    return type;
  }
  if(token.skip("*")) {
    while(token.skip("const") || token.skip("volatile"));
    return read_declarator(name, basety->getPointerTo(), param);
  }
  if(token.get().type == TOK_TYPE_IDENT) {
    name = token.next().val;
    return read_declarator_tail(basety, param);
  }
  return read_declarator_tail(basety, param);
}

llvm::Type *Parser::read_declarator_tail(llvm::Type *basety, std::vector<argument_t *> &param) {
  if(token.skip("[")) 
    return read_declarator_array(basety);
  if(token.skip("(")) {
    return read_declarator_func(basety, param);
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
  std::vector<argument_t *> _;
  llvm::Type *t = read_declarator_tail(basety, _);
  if(len == -1) return t->getPointerTo();
  return llvm::ArrayType::get(t, len);
}

llvm::Type *Parser::read_declarator_func(llvm::Type *basety, std::vector<argument_t *> &param) {
  param = read_declarator_param();

  bool has_vararg = false;
  std::vector<llvm::Type *> llvm_param;
  for(auto a : param) {
    if(a->type == nullptr) has_vararg = true;
    else llvm_param.push_back(a->type); 
  }
  llvm::FunctionType *llvm_func_type = 
    llvm::FunctionType::get(basety, llvm_param, has_vararg); 
  return llvm_func_type;
}

llvm::Type *Parser::read_func_param(std::string &name) {
  llvm::Type *basety = builder.getInt32Ty();
  if(is_type()) basety = read_type_spec();
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
  int stg;
  llvm::Type *ret_type = read_type_spec(stg);
  std::string name;// = token.next().val;
  std::vector<argument_t *> args;
  auto fty = static_cast<llvm::FunctionType *>(read_declarator(name, ret_type, args));
  std::vector<std::string> args_name = [&]() {
    std::vector<std::string> a;
    for(auto arg : args) {
      a.push_back(arg->name); 
    }
    return a;
  }();
  AST_vec body;
  token.expect_skip("{");
  while(!token.skip("}")) {
    auto st = statement();
    if(st) body.push_back(st);
    while(token.skip(";"));
  }
  return new FunctionDefAST(name, fty, args_name, body, stg);
}

AST *Parser::make_function_proto() {
  int stg;
  llvm::Type *ret_type = read_type_spec(stg);
  std::string name;
  llvm::FunctionType *fty = static_cast<llvm::FunctionType *>(read_declarator(name, ret_type));
  return new FunctionProtoAST(name, fty, stg);
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
  if(!t_strct) { // if not declared
    // create empty struct
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

llvm::Type *Parser::make_union_declaration() {
  if(!token.skip("union")) return nullptr;
  std::string name;

  if(token.get().type == TOK_TYPE_IDENT) 
    name = token.next().val;

  if(name.empty()) [](std::string &str) { // if name is empty, generate random name
    int len = 8; while(len--)
      str += (rand() % 26) + 65;
  }(name);

  llvm::StructType *new_union = nullptr;
  auto t_strct = this->union_list.get("union." + name);
  if(!t_strct) { // if not declared
    // create empty union
    new_union = llvm::StructType::create(context, "union." + name);
    this->union_list.add("union." + name, std::vector<union_elem_t>(), new_union);
    t_strct = this->union_list.get("union." + name);
  } else new_union = t_strct->llvm_union;
  // this if block should be function. not beautiful
  if(token.is("{")) {
    BlockAST *decls = (BlockAST *)statement();
    std::vector<union_elem_t> members;
    for(auto a : decls->body) {
      if(a->get_type() != AST_VAR_DECLARATION) error("error: union fields must be varaible declaration");
      VarDeclarationAST *decl = (VarDeclarationAST *)a;
      for(auto v : decl->decls)
        members.push_back(union_elem_t(v->name, v->type));
    }
    t_strct = this->union_list.get("union." + name);
    t_strct->members = members;

    Type_vec field;
    llvm::Type *last = nullptr;
    for(auto a : decls->body) {
      VarDeclarationAST *decl = (VarDeclarationAST *)a;
      for(auto v : decl->decls)
        if(last == nullptr) last = v->type;
        else if(data_layout->getTypeAllocSize(last) < data_layout->getTypeAllocSize(v->type))
          last = v->type;
    }
    field.push_back(last);
    if(new_union->isOpaque())
      new_union->setBody(field, false);
  } else if(token.is(";")) { // prototype (e.g. union A;)
  }
  return new_union;
}

llvm::Type *Parser::make_enum_declaration() {
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
  llvm::Type *basety = read_type_spec();
  std::string name; auto from = read_declarator(name, basety); 
  if(name.empty()) error("error(%d): expected identifier", token.get().line);
  typedef_map[name] = from;
  return;
}


bool Parser::is_function_proto() {
  int pos = token.pos;
  std::function<void()> skip_brackets = [&]() {
    while(1) {
      if(token.get().type == TOK_TYPE_END) error("error: program reached EOF");
      if(token.skip(")")) return;
      if(token.skip("(")) skip_brackets();
      else token.skip();
    }
  };

  bool f = false;
  for(;;) {
    if(token.skip(";")) break;
    if(is_type()) { token.skip(); continue; }
    if(token.skip("(")) { skip_brackets(); continue; }
    if(token.get().type != TOK_TYPE_IDENT) {
      token.skip(); continue; }
    token.skip(); // func name
    if(!token.skip("(")) break;//;continue;
    skip_brackets();
    f = token.skip(";");
    break;
  }
  token.pos = pos;
  return f;
}

bool Parser::is_function_def() {
  int pos = token.pos;
  std::function<void()> skip_brackets = [&]() {
    while(1) {
      if(token.get().type == TOK_TYPE_END) error("error: program reached EOF");
      if(token.skip(")")) return;
      if(token.skip("(")) skip_brackets();
      else token.skip();
    }
  };

  bool f = false;
  for(;;) {
    if(token.skip(";")) break;
    if(is_type()) { token.skip(); continue; }
    if(token.skip("(")) { skip_brackets(); continue; }
    if(token.get().type != TOK_TYPE_IDENT) {
      token.skip(); continue; }
    token.skip(); // func name
    if(!token.skip("(")) break;//;continue;
    skip_brackets();
    f = token.skip("{");
    break;
  }
  token.pos = pos;
  return f;
}

bool Parser::is_type() {
  auto cur = token.get().val;
  if(
      cur == "static"   ||
      cur == "const"    ||
      cur == "register" ||
      cur == "extern"   ||
      cur == "void"     ||
      cur == "unsigned" ||
      cur == "signed"   ||
      cur == "int"      ||
      cur == "char"     ||
      cur == "short"    ||
      cur == "long"     ||
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

llvm::Type *Parser::read_type_spec() {
  int d;
  return read_type_spec(d);
}

llvm::Type *Parser::read_type_spec(int &stg) {
  enum { tsigned, tunsigned } sign = tsigned;
  enum { tvoid, tchar, tint, tdouble } type = tint;
  enum { tshort, tdefault, tlong, tllong } size = tdefault;

  for(;;) {
    if(typedef_map.count(token.get().val)) {
      std::string t = token.next().val; return typedef_map[t];
    }
         if(token.skip("extern")) stg = STG_EXTERN;
    else if(token.skip("static")) stg = STG_STATIC;
    else if(token.skip("const")) ;//stg = STG_EXTERN;
    else if(token.skip("register")) ;// stg = STG_STATIC;

    // TODO: wanna use skip(), not is().
    else if(token.is("struct") ||
            token.is("union"))    return read_struct_union_type();
    else if(token.skip("enum"))   return read_enum_type();

    else if(token.skip("..."))    return nullptr;

    else if(token.skip("signed")) sign = tsigned;
    else if(token.skip("unsigned")) sign = tunsigned;

    else if(token.skip("void"))   type = tvoid;
    else if(token.skip("int"))    type = tint;
    else if(token.skip("char"))   type = tchar;
    else if(token.skip("double")) type = tdouble;
    else if(token.skip("short"))  size = tshort;
    else if(token.skip("long"))   {
      if(size == tlong) size = tllong;
      else size = tlong; 
    } else break;
  }

  switch(type) {
    case tvoid:   return builder.getVoidTy();
    case tchar:   return builder.getInt8Ty();
    case tdouble: return builder.getDoubleTy();
    default: break;
  }

  switch(size) {
    case tshort: return builder.getInt16Ty();
    case tlong:  return builder.getInt32Ty();
    case tllong: return builder.getInt64Ty();
    default:     return builder.getInt32Ty();
  }

  return nullptr;
}

llvm::Type *Parser::read_struct_union_type() {
  bool is_struct = token.is("struct");
  if( token.get(1).val == "{" || // struct { ... }
      token.get(2).val == "{" || // struct NAME { ... }
      token.get(2).val == ";") {//   struct NAME; (prototype?)
    if(is_struct) return make_struct_declaration();
    else return make_union_declaration();
  }
  token.skip(); // 'struct' or 'union'
  std::string name;
  if(token.get().type == TOK_TYPE_IDENT) 
    name = token.next().val; 
  else puts("err"); // TODO: add err check
  if(is_struct)
    return this->struct_list.get("struct." + name)->llvm_struct;
  else return this->union_list.get("union." + name)->llvm_union;
}

llvm::Type *Parser::read_enum_type() {
  if( token.get().val == "{" || // enum { ... }
      token.get(1).val == "{") // enum NAME { ... }
    return make_enum_declaration();
  if(token.get().type == TOK_TYPE_IDENT)
    token.skip();
  return builder.getInt32Ty();
}

