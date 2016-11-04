#include "codegen.hpp"

llvm::LLVMContext &context(llvm::getGlobalContext());
llvm::IRBuilder<> builder(context);
llvm::Module *mod;

void Codegen::run(AST_vec ast, std::string out_file_name, bool emit_llvm_ir) {
  Type ty;
  mod = new llvm::Module("QCC", context);
  for(auto st : ast) statement(st);
  if(emit_llvm_ir) mod->dump();
  std::string EC;
  llvm::raw_fd_ostream out(out_file_name.c_str(), EC, llvm::sys::fs::OpenFlags::F_RW);
  llvm::WriteBitcodeToFile(mod, out);
}

llvm::AllocaInst *Codegen::create_entry_alloca(llvm::Function *TheFunction, std::string &VarName, llvm::Type *type) {
  llvm::IRBuilder<> TmpB(&TheFunction->getEntryBlock(),
      TheFunction->getEntryBlock().begin());
  return TmpB.CreateAlloca(type == nullptr ? llvm::Type::getInt32Ty(context) : type, 0, VarName.c_str());
}

llvm::Type *Codegen::to_llvm_type(Type *type) {
  switch(type->get().type) {
    case TY_VOID:
      return builder.getVoidTy();
    case TY_INT:
      return builder.getInt32Ty();
    case TY_CHAR:
      return builder.getInt8Ty();
    case TY_TYPEDEF: {
      return typedef_map.count(type->get().user_type) ?
        typedef_map[type->get().user_type] : nullptr;
    }
    case TY_STRUCT_DEF: {
      return statement((StructDeclarationAST *)type->get().su);
    }
    case TY_STRUCT: {
      auto strct = this->struct_list.get("struct." + type->get().user_type);
      if(strct)
        return strct->llvm_struct;
      else error("error: not found struct type '%s'", strct->name.c_str());
    }
    case TY_PTR:
      return to_llvm_type(type->next)->getPointerTo();
    case TY_ARRAY: {
      if(type->get().ary_size == -1) {
        return to_llvm_type(type->next)->getPointerTo();
      } else 
        return llvm::ArrayType::get(to_llvm_type(type->next), type->get().ary_size);
    }
  }
  return nullptr;
}

llvm::Value *Codegen::type_cast(llvm::Value *val, llvm::Type *to) {
  if(val->getType()->isIntegerTy() && to->isIntegerTy()) {
    llvm::IntegerType *ival = (llvm::IntegerType *)val->getType();
    llvm::IntegerType *ito  = (llvm::IntegerType *)to;
    if(ival->getBitWidth() < ito->getBitWidth()) 
      return builder.CreateSExtOrBitCast(val, to);
  } 
  return builder.CreateTruncOrBitCast(val, to);
}

llvm::Value *Codegen::statement(AST *st) {
  switch(st->get_type()) {
    case AST_FUNCTION_DEF:
      return statement((FunctionDefAST *)st);
    case AST_FUNCTION_PROTO:
      return statement((FunctionProtoAST *)st);
    case AST_BLOCK:
      return statement((BlockAST *)st);
    case AST_FUNCTION_CALL:
      return statement((FunctionCallAST *)st);
    case AST_IF:
      return statement((IfAST *)st);
    case AST_WHILE:
      return statement((WhileAST *)st);
    case AST_FOR:
      return statement((ForAST *)st);
    case AST_RETURN:
      return statement((ReturnAST *)st);
    case AST_VAR_DECLARATION:
      return statement((VarDeclarationAST *)st);
    case AST_TYPEDEF:
      statement((TypedefAST *)st); break;
    case AST_STRUCT_DECLARATION:
      statement((StructDeclarationAST *)st); break;
    case AST_VARIABLE:
      return statement((VariableAST *)st);
    case AST_ASGMT:
      return statement((AsgmtAST *)st);
    case AST_INDEX:
      return statement((IndexAST *)st);
    case AST_UNARY:
      return statement((UnaryAST *)st);
    case AST_BINARY:
      return statement((BinaryAST *)st);
    case AST_DOT:
      return statement((DotOpAST *)st);
    case AST_STRING:
      return statement((StringAST *)st);
    case AST_NUMBER:
      return statement((NumberAST *)st);
  }
  return nullptr;
}

llvm::Value *Codegen::statement(FunctionProtoAST *st) {
  func_t func;
  func.name = st->name;
  func.ret_type = st->ret_type;
  std::vector<llvm::Type *> llvm_args_type;
  std::vector<Type *>       args_type;
  std::vector<std::string>  args_name;
  bool has_vararg = false;
  for(auto arg : st->args) {
    if(arg->eql(TY_VARARG)) {
      has_vararg = true;
    } else {
      func.args_type.push_back(arg);
      func.llvm_args_type.push_back(to_llvm_type(arg));
    }
  }
  this->func_list.add(func);
  func_t *function = this->func_list.get(func.name);
  
  llvm::FunctionType *llvm_func_type = 
    llvm::FunctionType::get(to_llvm_type(func.ret_type), func.llvm_args_type, has_vararg);
  llvm::Function *llvm_func = 
    llvm::Function::Create(llvm_func_type, llvm::Function::ExternalLinkage, func.name, mod);
  function->llvm_function = llvm_func;

  return nullptr;
}

llvm::Value *Codegen::statement(FunctionDefAST *st) {
  func_t func;
  func.name = st->name;
  func.ret_type = st->ret_type;
  std::vector<llvm::Type *> llvm_args_type;
  std::vector<Type *>       args_type;
  std::vector<std::string>  args_name;
  for(auto arg : st->args) {
    std::function<Type *(Type *)> ary_to_ptr = [&](Type *ty) -> Type * {
      if(ty->eql(TY_ARRAY)) {
        ty = new Type(TY_PTR, ary_to_ptr(ty->next));
      }
      return ty;
    }; // array(e.g. int a[3]) will be casted to pointer.
    auto ty = ary_to_ptr(arg->type);
    func.args_type.push_back(ty);
    func.args_name.push_back(arg->name);
    func.var_list.add(var_t(arg->name, ty));
    func.llvm_args_type.push_back(to_llvm_type(arg->type));
  }
  this->func_list.add(func);
  func_t *function = this->func_list.get(func.name);

  llvm::FunctionType *llvm_func_type = 
    llvm::FunctionType::get(to_llvm_type(func.ret_type), func.llvm_args_type, false);
  llvm::Function *llvm_func = llvm::Function::Create(llvm_func_type, llvm::Function::ExternalLinkage, func.name, mod);
  function->llvm_function = llvm_func;

  { // create function body
    llvm::BasicBlock *entry = llvm::BasicBlock::Create(context, "entry", function->llvm_function);
    builder.SetInsertPoint(entry);
    
    auto llvm_args_type_it = function->llvm_args_type.begin();
    auto      args_name_it = function->args_name.begin();
    for(auto arg_it = function->llvm_function->arg_begin(); arg_it != function->llvm_function->arg_end(); ++arg_it) {
      arg_it->setName(*args_name_it);
      llvm::AllocaInst *ainst = create_entry_alloca(function->llvm_function, *args_name_it, *llvm_args_type_it);
      builder.CreateStore(arg_it, ainst);
      var_t *v = function->var_list.get(*args_name_it);
      if(v) v->val = ainst;
      llvm_args_type_it++; args_name_it++;
    }

    cur_func = function;
    statement(st->body);
    for(auto it = cur_func->llvm_function->getBasicBlockList().begin(); 
        it != cur_func->llvm_function->getBasicBlockList().end(); ++it) {
      auto term = it->back().isTerminator();
      if(!term) {
        printf("warning: in function '%s': expected termination instruction such as 'return'\n", function->name.c_str());
        if(function->llvm_function->getReturnType()->isVoidTy())
          builder.CreateRetVoid();
        else builder.CreateRet(llvm::ConstantInt::get(function->llvm_function->getReturnType(), 0));
      }
    }
  }

  return nullptr;
}

llvm::Value *Codegen::statement(BlockAST *st) {
  for(auto a : st->body) 
    statement(a);
  return nullptr;
}

llvm::Value *Codegen::statement(FunctionCallAST *st) {
  func_t *func = this->func_list.get(st->name);
  if(func == nullptr) error("error: not found the function \'%s\'", st->name.c_str());
  std::vector<llvm::Value *> caller_args;
  int i = 0;
  for(auto a : st->args) {
    caller_args.push_back(
        func->llvm_args_type.size() <= i? statement(a) : // varaible argument                                    
        type_cast(statement(a), func->llvm_args_type[i++])
        );
  } 
  auto callee = func->llvm_function;
  auto ret = builder.CreateCall(callee, caller_args);
  if(!callee->getReturnType()->isVoidTy())
    return ret;
  return nullptr;
}

llvm::Type *Codegen::statement(TypedefAST *st) {
  if(st->from->eql(TY_STRUCT)) {
    auto strct = this->struct_list.get("struct." + st->from->get().user_type);
    typedef_map.insert(std::make_pair(st->to, strct->llvm_struct));
    return strct->llvm_struct;
  } else if(st->from->eql(TY_STRUCT_DEF)) {
    auto strct = statement((StructDeclarationAST *)st->from->get().su);
    typedef_map[st->to] = strct;
    return strct;
  } else { // primitive type
    typedef_map[st->to] = to_llvm_type(st->from);
  }
  return nullptr;
}

llvm::Type *Codegen::statement(StructDeclarationAST *st) {
  if(st->def) return this->struct_list.get("struct." + st->name)->llvm_struct;
  st->def = true;
  std::vector<llvm::Type *> field;
  std::vector<std::string> members_name;
	if(st->name.empty()) [](std::string &str) {
		int len = 8; while(len--)
      str += (rand() % 26) + 65;
	}(st->name);
  llvm::StructType *new_struct = llvm::StructType::create(context, "struct." + st->name);
  BlockAST *decl_block = (BlockAST *)st->decls;
  // TODO: here's code is not beautiful :(
  for(auto a : decl_block->body) {
    if(a->get_type() == AST_VAR_DECLARATION) {
      VarDeclarationAST *decl = (VarDeclarationAST *)a;
      for(auto v : decl->decls)
        members_name.push_back(v->name);
    }
  }
  this->struct_list.add("struct." + st->name, members_name, new_struct);
  for(auto a : decl_block->body) {
    if(a->get_type() == AST_VAR_DECLARATION) {
      VarDeclarationAST *decl = (VarDeclarationAST *)a;
      for(auto v : decl->decls)
        field.push_back(to_llvm_type(v->type));
    }
  }
  if(new_struct->isOpaque())
    new_struct->setBody(field, false);
  return new_struct;
}

llvm::Value *Codegen::statement(VarDeclarationAST *st) {
  for(auto v : st->decls) {
    cur_func->var_list.add(var_t(v->name, v->type));
    auto cur_var = cur_func->var_list.get(v->name);
    llvm::Type *decl_type = to_llvm_type(cur_var->type);
    cur_var->val = builder.CreateAlloca(decl_type);
    if(v->init_expr) 
      asgmt_value(cur_var->val, statement(v->init_expr));
  }
  return nullptr;
}

llvm::Value *Codegen::statement(IfAST *st) {
  llvm::Value *cond_val = statement(st->cond);
  cond_val = builder.CreateICmpNE(cond_val, llvm::ConstantInt::get(cond_val->getType(), 0, true));

  auto *func = builder.GetInsertBlock()->getParent();

  llvm::BasicBlock *bb_then = llvm::BasicBlock::Create(context, "then", func);
  llvm::BasicBlock *bb_else = llvm::BasicBlock::Create(context, "else");
  llvm::BasicBlock *bb_merge= llvm::BasicBlock::Create(context, "merge");

  builder.CreateCondBr(cond_val, bb_then, bb_else);
  builder.SetInsertPoint(bb_then);

  bool necessary_merge = false;
  cur_func->br_list.push(false);
  statement(st->b_then);
  if(cur_func->br_list.top())
    cur_func->br_list.pop();
  else builder.CreateBr(bb_merge), necessary_merge = true;
  bb_then = builder.GetInsertBlock();

  func->getBasicBlockList().push_back(bb_else);
  builder.SetInsertPoint(bb_else);

  cur_func->br_list.push(false);
  if(st->b_else) statement(st->b_else);
  if(cur_func->br_list.top())
    cur_func->br_list.pop();
  else builder.CreateBr(bb_merge), necessary_merge = true;
  bb_else = builder.GetInsertBlock();

  if(necessary_merge) {
    func->getBasicBlockList().push_back(bb_merge);
    builder.SetInsertPoint(bb_merge);
  }
  
  return nullptr;
}

llvm::Value *Codegen::statement(WhileAST *st) {
  llvm::BasicBlock *bb_loop = llvm::BasicBlock::Create(context, "loop", cur_func->llvm_function);
  llvm::BasicBlock *bb_after_loop = llvm::BasicBlock::Create(context, "after_loop", cur_func->llvm_function);

  llvm::Value *cond_val_1 = statement(st->cond);
  llvm::Value *first_cond_val = builder.CreateICmpNE(
      cond_val_1, llvm::ConstantInt::get(cond_val_1->getType(), 0, true));
  builder.CreateCondBr(first_cond_val, bb_loop, bb_after_loop);

  builder.SetInsertPoint(bb_loop);

  statement(st->body);

  llvm::Value *cond_val_2 = statement(st->cond);
  llvm::Value *second_cond_val = builder.CreateICmpNE(
      cond_val_2, llvm::ConstantInt::get(cond_val_2->getType(), 0, true));
  builder.CreateCondBr(second_cond_val, bb_loop, bb_after_loop);

  builder.SetInsertPoint(bb_after_loop);

  return nullptr;
}

llvm::Value *Codegen::statement(ForAST *st) {
  auto func = builder.GetInsertBlock()->getParent();
  llvm::BasicBlock *bb_before_loop = llvm::BasicBlock::Create(context, "before_loop", func);
  llvm::BasicBlock *bb_loop = llvm::BasicBlock::Create(context, "loop", func);
  llvm::BasicBlock *bb_after_loop = llvm::BasicBlock::Create(context, "after_loop", func);

  statement(st->init);

  builder.CreateBr(bb_before_loop);
  builder.SetInsertPoint(bb_before_loop);
  llvm::Value *first_cond_val = builder.CreateICmpNE(
      statement(st->cond), llvm::ConstantInt::get(builder.getInt1Ty(), 0, true));
  builder.CreateCondBr(first_cond_val, bb_loop, bb_after_loop);
  builder.SetInsertPoint(bb_loop);

  statement(st->body);

  statement(st->reinit);

  builder.CreateBr(bb_before_loop);

  builder.SetInsertPoint(bb_after_loop);

  return nullptr;
}

llvm::Value *Codegen::statement(ReturnAST *st) {
  if(!cur_func->br_list.empty()) cur_func->br_list.top() = true;
  if(st->expr) 
    return builder.CreateRet(statement(st->expr));
  else
    return builder.CreateRetVoid();
}

llvm::Value *Codegen::statement(VariableAST *st) {
  auto var = this->cur_func->var_list.get(st->name);
  if(var) {
    if(var->type->eql(TY_ARRAY)) {
      llvm::Value *a = var->val;
      llvm::Value *elem = llvm::GetElementPtrInst::CreateInBounds(
          a, 
          llvm::ArrayRef<llvm::Value *>{
            llvm::ConstantInt::get(builder.getInt32Ty(), 0), 
            llvm::ConstantInt::get(builder.getInt32Ty(), 0)}, "elem", builder.GetInsertBlock());
      return elem;
    } else 
      return builder.CreateLoad(var->val);
  }
  error("error: not found variable '%s'", var->name.c_str());
  return nullptr;
}

llvm::Value *Codegen::get_value(AST *st) {
  if(st->get_type() == AST_VARIABLE) {
    VariableAST *va = (VariableAST *)st;
    auto cur_var = cur_func->var_list.get(va->name);
    return cur_var->val;
  } else if(st->get_type() == AST_INDEX) {
    IndexAST *vidx = (IndexAST *)st;
    llvm::Value *elem = get_element_ptr(vidx);
    return elem;
  } else if(st->get_type() == AST_UNARY) {
    UnaryAST *ua = (UnaryAST *)st;
    return statement(ua->expr);
  } else if(st->get_type() == AST_INDEX) {
    return get_element_ptr((IndexAST *)st);
  } else if(st->get_type() == AST_DOT) {
    DotOpAST *da = (DotOpAST *)st;
    auto parent = get_value(da->lhs);
    if(da->is_arrow) parent = builder.CreateLoad(parent);
    if(!parent) error("error");
    struct_t *sinfo = this->struct_list.get(
        ((llvm::StructType *)parent->getType())->getPointerElementType()->getStructName().str()
        );
    if(!sinfo) error("error: not found");
    auto strct = sinfo->llvm_struct;
    int member_count = 0;
    std::string expected_name = ((VariableAST *)da->rhs)->name;
    for(auto m : sinfo->members_name) {
      if(m == expected_name) break;
      member_count++;
    }
    return builder.CreateStructGEP(parent, member_count);
  }
  return nullptr;
}

llvm::Value *Codegen::get_element_ptr(IndexAST *st) {
  llvm::Value *a = nullptr;
  bool ptr = false;
  if(st->ary->get_type() == AST_VARIABLE) { 
    VariableAST *va = (VariableAST *)st->ary;
    auto v = this->cur_func->var_list.get(va->name);
    a = v->type->eql(TY_PTR) ? ptr = true, builder.CreateLoad(v->val) : v->val;
  } else if(st->ary->get_type() == AST_INDEX) {
    a = get_element_ptr((IndexAST *)st->ary);
    if(!a->getType()->getArrayElementType()->isArrayTy()) {
      ptr = true;
      a = builder.CreateLoad(a);
    }
  }
  llvm::Value *elem;
  if(ptr) {
    elem = llvm::GetElementPtrInst::CreateInBounds(
        a, 
        llvm::ArrayRef<llvm::Value *>(
        statement(st->idx)), "elem", builder.GetInsertBlock());
  } else {
    elem = llvm::GetElementPtrInst::CreateInBounds(
        a, 
        llvm::ArrayRef<llvm::Value *>{llvm::ConstantInt::get(builder.getInt32Ty(), 0), 
        statement(st->idx)}, "elem", builder.GetInsertBlock());
  }
  return elem;
}

llvm::Value *Codegen::asgmt_value(llvm::Value *dst, llvm::Value *src) {
  src = this->type_cast(src, dst->getType()->getPointerElementType());
  return builder.CreateStore(src, dst);
}

llvm::Value *Codegen::statement(AsgmtAST *st) {
  auto src = statement(st->src);
  llvm::Value *dst = nullptr;
  dst = get_value(st->dst);
  asgmt_value(dst, src);
  return builder.CreateLoad(dst);
}

llvm::Value *Codegen::statement(IndexAST *st) {
  auto elem = get_element_ptr(st);
  return builder.CreateLoad(elem);
}

llvm::Value *Codegen::statement(UnaryAST *st) {
  if(st->op == "&") { // address
    return get_value(st->expr);
  } else if(st->op == "*") {
    auto e = statement(st->expr);
    return builder.CreateLoad(e);
  } else if(st->op == "++") {
    auto v1 = get_value(st->expr);
    auto v  = builder.CreateLoad(v1);
    llvm::Value *vv;
    if(v->getType()->isPointerTy()) {
      vv = llvm::GetElementPtrInst::CreateInBounds(
          v, 
          llvm::ArrayRef<llvm::Value *>(
            llvm::ConstantInt::get(builder.getInt32Ty(), 1)), "elem", builder.GetInsertBlock());
    } else vv = builder.CreateAdd(v, llvm::ConstantInt::get(v->getType(), 1));
    asgmt_value(v1, vv);
    return st->postfix ? v : vv;
  } else if(st->op == "--") {
    auto v1 = get_value(st->expr);
    auto v  = builder.CreateLoad(v1);
    auto vv= builder.CreateSub(v, llvm::ConstantInt::get(v->getType(), 1));
    asgmt_value(v1, vv);
    return st->postfix ? v : vv;
  }
  return nullptr;
}

llvm::Value *Codegen::statement(BinaryAST *st) {
  auto lhs = statement(st->lhs),
       rhs = statement(st->rhs);
  llvm::Value *ret = nullptr;
  if(st->op == "+") {
    ret = builder.CreateAdd(lhs, rhs);
  } else if(st->op == "-") {
    ret = builder.CreateSub(lhs, rhs);
  } else if(st->op == "*") {
    ret = builder.CreateMul(lhs, rhs);
  } else if(st->op == "/") {
    ret = builder.CreateSDiv(lhs, rhs);
  } else if(st->op == "%") {
    ret = builder.CreateSRem(lhs, rhs);
  } else if(st->op == "==") {
    ret = builder.CreateICmpEQ(lhs, rhs);
  } else if(st->op == "!=") {
    ret = builder.CreateICmpNE(lhs, rhs);
  } else if(st->op == "<=") {
    ret = builder.CreateICmpSLE(lhs, rhs);
  } else if(st->op == (">=")) {
    ret = builder.CreateICmpSGE(lhs, rhs);
  } else if(st->op == "<") {
    ret = builder.CreateICmpSLT(lhs, rhs);
  } else if(st->op == (">")) {
    ret = builder.CreateICmpSGT(lhs, rhs);
  } else if(st->op == "&" || st->op == "&&") {
    ret = builder.CreateAnd(lhs, rhs);
  } else if(st->op == "|" || st->op == "||") {
    ret = builder.CreateOr(lhs, rhs);
  } else if(st->op == "^") {
    ret = builder.CreateXor(lhs, rhs);
  }
  return ret;
}

llvm::Value *Codegen::statement(DotOpAST *st) {
  return builder.CreateLoad(get_value(st));
}

llvm::Value *Codegen::statement(StringAST *st) {
  return builder.CreateGlobalStringPtr(st->str);
}

llvm::Value *Codegen::statement(NumberAST *st) {
  return llvm::ConstantInt::get(builder.getInt32Ty(), st->number, true);
}

void Codegen::error(const char *errs, ...) {
  va_list args;
  va_start(args, errs);
    vprintf(errs, args); puts("");
  va_end(args);
  exit(0);
}
