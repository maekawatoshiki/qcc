#include "codegen.hpp"

llvm::LLVMContext &context(llvm::getGlobalContext());
llvm::IRBuilder<> builder(context);
llvm::Module *mod;

void Codegen::run(AST_vec ast, std::string out_file_name, bool emit_llvm_ir) {
  mod = new llvm::Module("QCC", context);
  data_layout = new llvm::DataLayout(mod);
  { // create function(s) used in array initialization
    llvm::FunctionType *llvm_func_type_memcpy = 
      llvm::FunctionType::get(
          builder.getVoidTy(),
          std::vector<llvm::Type *>{
          builder.getInt8PtrTy(), 
          builder.getInt8PtrTy(),
          builder.getInt32Ty(),
          builder.getInt32Ty(),
          builder.getInt1Ty()}, /*var arg=*/false);
    tool_memcpy = llvm::Function::Create(llvm_func_type_memcpy, 
        llvm::Function::ExternalLinkage, "llvm.memcpy.p0i8.p0i8.i32", mod);
  }
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

llvm::Value *Codegen::type_cast(llvm::Value *val, llvm::Type *to) {
  if(val->getType()->isIntegerTy() && to->isIntegerTy()) {
    llvm::IntegerType *ival = (llvm::IntegerType *)val->getType();
    llvm::IntegerType *ito  = (llvm::IntegerType *)to;
    if(ival->getBitWidth() < ito->getBitWidth()) 
      return builder.CreateZExtOrBitCast(val, to);
  } else if(val->getType()->isIntegerTy() && to->isDoubleTy()) {
    return builder.CreateSIToFP(val, to);
  } else if(val->getType()->isDoubleTy() && to->isIntegerTy()) {
    return builder.CreateFPToSI(val, to);
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
    case AST_BREAK:
      return statement((BreakAST *)st);
    case AST_CONTINUE:
      return statement((ContinueAST *)st);
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
    case AST_VARIABLE:
      return statement((VariableAST *)st);
    case AST_ASGMT:
      return statement((AsgmtAST *)st);
    case AST_INDEX:
      return statement((IndexAST *)st);
    case AST_ARRAY:
      return statement((ArrayAST *)st);
    case AST_TYPECAST:
      return statement((TypeCastAST *)st);
    case AST_UNARY:
      return statement((UnaryAST *)st);
    case AST_BINARY:
      return statement((BinaryAST *)st);
    case AST_TERNARY:
      return statement((TernaryAST *)st);
    case AST_DOT:
      return statement((DotOpAST *)st);
    case AST_SIZEOF:
      return statement((SizeofAST *)st);
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
  std::vector<std::string>  args_name;
  bool has_vararg = false;
  for(auto arg : st->args) {
    if(arg->type == nullptr) { // null means variable arguments
      has_vararg = true;
    } else {
      func.llvm_args_type.push_back(arg->type);
    }
  }
  this->func_list.add(func);
  func_t *function = this->func_list.get(func.name);
  
  llvm::FunctionType *llvm_func_type = 
    llvm::FunctionType::get(func.ret_type, func.llvm_args_type, has_vararg);
  llvm::Function *llvm_func = 
    llvm::Function::Create(llvm_func_type, llvm::Function::ExternalLinkage, func.name, mod);
  function->llvm_function = llvm_func;

  return nullptr;
}

llvm::Value *Codegen::statement(FunctionDefAST *st) {
  func_t func;
  // if prototype exists, use it instead
  func_t *proto = this->func_list.get(st->name);
  if(proto) func = *proto;
  else {
    func.name = st->name;
    func.ret_type = st->ret_type;
  }
  std::vector<llvm::Type *> llvm_args_type;
  std::vector<std::string>  args_name;

  func.block_list.create_new_block();
  for(auto arg : st->args) {
    func.args_name.push_back(arg->name);
    func.block_list.get_varlist()->add(var_t(arg->name, arg->type));
    func.llvm_args_type.push_back(arg->type);
  }
  this->func_list.add(func);
  func_t *function = this->func_list.get(func.name);

  if(!proto) {
    llvm::FunctionType *llvm_func_type = 
      llvm::FunctionType::get(func.ret_type, func.llvm_args_type, false);
    llvm::Function *llvm_func = llvm::Function::Create(llvm_func_type, llvm::Function::ExternalLinkage, func.name, mod);
    function->llvm_function = llvm_func;
  }

  { // create function body
    llvm::BasicBlock *entry = llvm::BasicBlock::Create(context, "entry", function->llvm_function);
    builder.SetInsertPoint(entry);
    
    cur_func = function;
    auto llvm_args_type_it = function->llvm_args_type.begin();
    auto      args_name_it = function->args_name.begin();
    for(auto arg_it = function->llvm_function->arg_begin(); arg_it != function->llvm_function->arg_end(); ++arg_it) {
      arg_it->setName(*args_name_it);
      llvm::AllocaInst *ainst = create_entry_alloca(function->llvm_function, *args_name_it, *llvm_args_type_it);
      builder.CreateStore(arg_it, ainst);
      var_t *v = lookup_var(*args_name_it);
      if(v) v->val = ainst;
      llvm_args_type_it++; args_name_it++;
    }

    for(auto stmt : st->body)
      statement(stmt);
    for(auto it = cur_func->llvm_function->getBasicBlockList().begin(); 
        it != cur_func->llvm_function->getBasicBlockList().end(); ++it) {
      auto term = !it->empty();
      if(term) term = it->back().isTerminator();
      if(!term) {
        printf("warning: in function '%s': expected termination instruction such as 'return'\n", function->name.c_str());
        if(function->llvm_function->getReturnType()->isVoidTy())
          builder.CreateRetVoid();
        else {
          auto ret_type = function->llvm_function->getReturnType();
          if(ret_type->isDoubleTy())
            builder.CreateRet(llvm::ConstantFP::get(builder.getDoubleTy(), 0.0));
          else // ptr, int ...
            builder.CreateRet(make_int(0, function->llvm_function->getReturnType()));
        }
      }
    }
    cur_func = nullptr;
  }

  return nullptr;
}

llvm::Value *Codegen::statement(BlockAST *st) {
  this->cur_func->block_list.create_new_block();
  for(auto a : st->body) 
    statement(a);
  this->cur_func->block_list.escape_block();
  return nullptr;
}

llvm::Value *Codegen::statement(FunctionCallAST *st) {
  llvm::Function *f = st->callee->get_type() == AST_VARIABLE ? // function ptr or function name
    [&]() -> llvm::Function * {
      VariableAST *va = static_cast<VariableAST *>(st->callee);
      func_t *func = this->func_list.get(va->name);
      if(!func) { // not function, it is variable of function pointer
        auto var = lookup_var(va->name);
        if(!var) error("error: not found the function \'%s\'", va->name.c_str());
        return (llvm::Function *)builder.CreateLoad(var->val);
      } else return func->llvm_function; 
    }() : reinterpret_cast<llvm::Function *>( statement(st->callee) );

  int params = f->getType()->getPointerElementType()->getFunctionNumParams();
  std::vector<llvm::Value *> caller_args;
  int i = 0;
  for(auto a : st->args) {
    caller_args.push_back(
        params <= i ? statement(a) : // varaible argument                                    
        type_cast(statement(a), f->getType()->getPointerElementType()->getFunctionParamType(i++))
        );
  } 
  auto callee = (llvm::Function *)f;
  auto ret = builder.CreateCall(callee, caller_args);
  if(!callee->getReturnType()->isVoidTy())
    return ret;
  return nullptr;
}

llvm::Value *Codegen::statement(VarDeclarationAST *st) {
  for(auto v : st->decls) {
    llvm::Value *init_val = nullptr;
    if(v->init_expr) init_val = statement(v->init_expr);
    // int a[] = {1, 2}; -->> int a[2] = {1, 2};
    if(v->type->isPointerTy() && init_val && init_val->getType()->isArrayTy()) v->type = init_val->getType();
    if(cur_func == nullptr) { // global 
      global_var.add(var_t(v->name, v->type));
      auto cur_var = lookup_var(v->name);
      mod->getOrInsertGlobal(v->name, v->type);
      llvm::GlobalVariable *gv = mod->getNamedGlobal(v->name);
      if(v->init_expr) {
        auto expr = statement(v->init_expr);
        if(llvm::Constant *c = llvm::dyn_cast<llvm::Constant>(expr)) {
          if(c->getType()->isArrayTy() && 
              c->getType()->getArrayNumElements() != gv->getType()->getPointerElementType()->getArrayNumElements()) {
            ArrayAST *ar = (ArrayAST *)v->init_expr;
            c = create_const_array(ar->elems, gv->getType()->getPointerElementType()->getArrayNumElements());
          }
          gv->setInitializer(c);
          gv->setLinkage(llvm::GlobalVariable::ExternalLinkage);
        } else error("error: initialization of global variables must be constant");
      } else {
        gv->setLinkage(llvm::GlobalVariable::CommonLinkage);
        llvm::ConstantAggregateZero *zeroinit = llvm::ConstantAggregateZero::get(gv->getType()->getElementType());
        gv->setInitializer(zeroinit);
      }
      cur_var->val = gv;
    } else {
      cur_func->block_list.get_varlist()->add(var_t(v->name, v->type));
      auto cur_var = lookup_var(v->name);
      if(!cur_var) error("error: not found variable '%s'", v->name.c_str());
      llvm::IRBuilder<> B = [&]() -> llvm::IRBuilder<> {
        if(cur_func->llvm_function->begin()->empty())
          return llvm::IRBuilder<>(cur_func->llvm_function->begin());
        else return llvm::IRBuilder<>(cur_func->llvm_function->begin()->begin());
      }();
      cur_var->val = B.CreateAlloca(cur_var->type, nullptr, cur_var->name);
      if(init_val) 
        asgmt_value(cur_var->val, init_val);
    }
  }
  return nullptr;
}

llvm::Value *Codegen::statement(BreakAST *st) {
  cur_func->br_list.top() = true;
  return builder.CreateBr(cur_func->break_list.top());
}
llvm::Value *Codegen::statement(ContinueAST *st) {
  cur_func->br_list.top() = true;
  return builder.CreateBr(cur_func->continue_list.top());
}

llvm::Value *Codegen::statement(IfAST *st) {
  llvm::Value *val_cond = statement(st->cond);
  val_cond = builder.CreateICmpNE(
      val_cond, 
      val_cond->getType()->isPointerTy() ?
      llvm::ConstantPointerNull::getNullValue(val_cond->getType()) :
      make_int(0, val_cond->getType()));

  auto *func = builder.GetInsertBlock()->getParent();

  llvm::BasicBlock *bb_then = llvm::BasicBlock::Create(context, "then", func);
  llvm::BasicBlock *bb_else = llvm::BasicBlock::Create(context, "else");
  llvm::BasicBlock *bb_merge= llvm::BasicBlock::Create(context, "merge");

  builder.CreateCondBr(val_cond, bb_then, bb_else);
  builder.SetInsertPoint(bb_then);

  bool necessary_merge = false, has_br;
  cur_func->br_list.push(false);
  statement(st->b_then);
  if(cur_func->br_list.top());
  else builder.CreateBr(bb_merge), necessary_merge = true;
  has_br = cur_func->br_list.top();
  cur_func->br_list.pop();
  if(!cur_func->br_list.empty()) cur_func->br_list.top() = has_br;
  bb_then = builder.GetInsertBlock();

  func->getBasicBlockList().push_back(bb_else);
  builder.SetInsertPoint(bb_else);

  cur_func->br_list.push(false);
  if(st->b_else) statement(st->b_else);
  if(cur_func->br_list.top());
  else builder.CreateBr(bb_merge), necessary_merge = true;
  has_br = cur_func->br_list.top();
  cur_func->br_list.pop();
  if(!cur_func->br_list.empty()) cur_func->br_list.top() = has_br;
  bb_else = builder.GetInsertBlock();

  if(necessary_merge) {
    func->getBasicBlockList().push_back(bb_merge);
    builder.SetInsertPoint(bb_merge);
  }
  
  return nullptr;
}

llvm::Value *Codegen::statement(WhileAST *st) {
  llvm::BasicBlock *bb_before_loop = llvm::BasicBlock::Create(context, "before_loop", cur_func->llvm_function);
  llvm::BasicBlock *bb_loop = llvm::BasicBlock::Create(context, "loop", cur_func->llvm_function);
  llvm::BasicBlock *bb_after_loop = llvm::BasicBlock::Create(context, "after_loop", cur_func->llvm_function);

  builder.CreateBr(bb_before_loop);

  builder.SetInsertPoint(bb_before_loop);
  llvm::Value *val_cond_1 = statement(st->cond);
  llvm::Value *first_val_cond = builder.CreateICmpNE(
      val_cond_1, 
      val_cond_1->getType()->isPointerTy() ?
      llvm::ConstantPointerNull::getNullValue(val_cond_1->getType()) :
      llvm::ConstantInt::get(val_cond_1->getType(), 0, true));
  builder.CreateCondBr(first_val_cond, bb_loop, bb_after_loop);

  builder.SetInsertPoint(bb_loop);

  cur_func->break_list.push(bb_after_loop);
  cur_func->continue_list.push(bb_before_loop);
  statement(st->body);
  cur_func->break_list.pop();
  cur_func->continue_list.pop();

  builder.CreateBr(bb_before_loop);

  builder.SetInsertPoint(bb_after_loop);

  return nullptr;
}

llvm::Value *Codegen::statement(ForAST *st) {
  auto func = builder.GetInsertBlock()->getParent();
  llvm::BasicBlock *bb_before_loop = llvm::BasicBlock::Create(context, "before_loop", func);
  llvm::BasicBlock *bb_loop = llvm::BasicBlock::Create(context, "loop", func);
  llvm::BasicBlock *bb_step = llvm::BasicBlock::Create(context, "loop_step", func);
  llvm::BasicBlock *bb_after_loop = llvm::BasicBlock::Create(context, "after_loop", func);

  statement(st->init);

  builder.CreateBr(bb_before_loop);
  builder.SetInsertPoint(bb_before_loop);
  auto val_cond = statement(st->cond);
  llvm::Value *first_val_cond = builder.CreateICmpNE(
      val_cond, 
      val_cond->getType()->isPointerTy() ?
      llvm::ConstantPointerNull::getNullValue(val_cond->getType()) :
      llvm::ConstantInt::get(val_cond->getType(), 0, true));
  builder.CreateCondBr(first_val_cond, bb_loop, bb_after_loop);
  builder.SetInsertPoint(bb_loop);

  cur_func->break_list.push(bb_after_loop);
  cur_func->continue_list.push(bb_step);
  statement(st->body);
  cur_func->break_list.pop();
  cur_func->continue_list.pop();
  builder.CreateBr(bb_step);

  builder.SetInsertPoint(bb_step);
  statement(st->reinit);

  builder.CreateBr(bb_before_loop);

  builder.SetInsertPoint(bb_after_loop);

  return nullptr;
}

llvm::Value *Codegen::statement(ReturnAST *st) {
  if(!cur_func->br_list.empty()) cur_func->br_list.top() = true;
  if(st->expr) 
    return builder.CreateRet(type_cast(statement(st->expr), cur_func->llvm_function->getReturnType()));
  else
    return builder.CreateRetVoid();
}

llvm::Value *Codegen::statement(VariableAST *st) {
  auto var = lookup_var(st->name);
  if(var) {
    if(var->type->isArrayTy()) {
      llvm::Value *a = var->val;
      llvm::Value *elem = llvm::GetElementPtrInst::CreateInBounds(
          a, 
          std::vector<llvm::Value *>{
            llvm::ConstantInt::get(builder.getInt32Ty(), 0), 
            llvm::ConstantInt::get(builder.getInt32Ty(), 0)}, "elem", builder.GetInsertBlock());
      return elem;
    } else 
      return builder.CreateLoad(var->val);
  } else { // function name?
    auto f = func_list.get(st->name);
    if(f) return f->llvm_function;
  }
  error("error: not found variable '%s'", st->name.c_str());
  return nullptr;
}

llvm::Value *Codegen::get_value(AST *st) {
  if(st->get_type() == AST_VARIABLE) {
    VariableAST *va = (VariableAST *)st;
    auto cur_var = lookup_var(va->name);
    if(!cur_var) error("error: not found variable '%s'", va->name.c_str());
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
    auto parent = statement(da->lhs);
    if(!parent->getType()->isPointerTy())
      parent = get_value(da->lhs);
    if(!parent) error("error: " __FILE__ "(%d)", __LINE__);
    struct_t *sinfo = this->struct_list.get(
        ((llvm::StructType *)parent->getType())->getPointerElementType()->getStructName().str()
        );
    if(!sinfo) error("error: not found struct");
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

llvm::Constant *Codegen::create_const_array(std::vector<AST *> ast_elems, int size) {
  std::vector<llvm::Constant *> elems;
  for(auto elem : ast_elems) {
    elems.push_back( (llvm::Constant *)statement(elem) );
  }
  for(int i = elems.size(); i < size; i++)
    elems.push_back(llvm::ConstantInt::get(elems[0]->getType(), 0));
  auto ary_type = llvm::ArrayType::get(elems[0]->getType(), elems.size());
  return llvm::ConstantArray::get(ary_type, elems);
}

llvm::Value *Codegen::get_element_ptr(IndexAST *st) {
  llvm::Value *a = nullptr;
  bool ptr = false;
  if(st->ary->get_type() == AST_VARIABLE) { 
    VariableAST *va = (VariableAST *)st->ary;
    auto v = lookup_var(va->name);
    if(!v) error("error: not found variable '%s'", va->name.c_str());
    a = v->type->isPointerTy() ? ptr = true, builder.CreateLoad(v->val) : v->val;
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
        std::vector<llvm::Value *>{llvm::ConstantInt::get(builder.getInt32Ty(), 0), 
        statement(st->idx)}, "elem", builder.GetInsertBlock());
  }
  return elem;
}

llvm::Value *Codegen::asgmt_value(llvm::Value *dst, llvm::Value *src) {
  if(dst->getType()->getPointerElementType()->isArrayTy()) {
    if(src->getType()->isArrayTy() && llvm::dyn_cast<llvm::Constant>(src)) {
      // if src is constant array, copy it to global variable
      std::string name = []() -> std::string { 
        std::string str;
        int len = 8; while(len--)
          str += (rand() % 26) + 65;
        return str;
      }();
      mod->getOrInsertGlobal("const_ary."+name, src->getType());
      llvm::GlobalVariable *gv = mod->getNamedGlobal("const_ary."+name);
      gv->setInitializer((llvm::Constant *)src);
      src = gv;
    }

    return builder.CreateCall(tool_memcpy,
        std::vector<llvm::Value *> {
          type_cast(dst, builder.getInt8Ty()->getPointerTo()), type_cast(src, builder.getInt8Ty()->getPointerTo()), 
          make_int(data_layout->getTypeAllocSize(src->getType()->getPointerElementType())),
          make_int(data_layout->getTypeAllocSize(
            [&]() -> llvm::Type * {
                auto basety = src->getType()->getPointerElementType();
                while(basety->isArrayTy()) basety = basety->getArrayElementType();
                return basety;
            }())), make_int(0, builder.getInt1Ty())});
  } else 
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

llvm::Value *Codegen::make_int(int n, llvm::Type *ty) {
  return llvm::ConstantInt::get(ty, n);
}

#define IMPLICIT_CAST \
  if(lhs->getType()->isDoubleTy()) \
    rhs = type_cast(rhs, builder.getDoubleTy()); \
  else if(rhs->getType()->isDoubleTy()) \
    lhs = type_cast(lhs, builder.getDoubleTy());

llvm::Value *Codegen::op_add(llvm::Value *lhs, llvm::Value *rhs) {
  IMPLICIT_CAST;
  if(lhs->getType()->isPointerTy() && rhs->getType()->isIntegerTy()) {
    return llvm::GetElementPtrInst::CreateInBounds(
        lhs, 
        llvm::ArrayRef<llvm::Value *>(rhs), "elem", builder.GetInsertBlock());
  } else if(lhs->getType()->isIntegerTy() && rhs->getType()->isIntegerTy()) {
    return builder.CreateAdd(lhs, type_cast(rhs, lhs->getType()));
  } else if(lhs->getType()->isDoubleTy()) {
    return builder.CreateFAdd(lhs, rhs);
  } else error("error: unknown operation");
  return nullptr;
}
llvm::Value *Codegen::op_sub(llvm::Value *lhs, llvm::Value *rhs) {
  IMPLICIT_CAST;
  if(lhs->getType()->isPointerTy() && rhs->getType()->isIntegerTy()) {
    return llvm::GetElementPtrInst::CreateInBounds(lhs,
        llvm::ArrayRef<llvm::Value *>(
          builder.CreateSub(make_int(0, rhs->getType()), rhs)), "elem", builder.GetInsertBlock());
  } else if(lhs->getType()->isIntegerTy() && rhs->getType()->isIntegerTy()) {
    return builder.CreateSub(lhs, type_cast(rhs, lhs->getType()));
  } else if(lhs->getType()->isDoubleTy()) {
    return builder.CreateFSub(lhs, rhs);
  } else error("error: unknown operation");
  return nullptr;
}
llvm::Value *Codegen::op_mul(llvm::Value *lhs, llvm::Value *rhs) {
  IMPLICIT_CAST;
  if(lhs->getType()->isIntegerTy() && rhs->getType()->isIntegerTy()) {
    return builder.CreateMul(lhs, type_cast(rhs, lhs->getType()));
  } else if(lhs->getType()->isDoubleTy()) {
    return builder.CreateFMul(lhs, rhs);
  } else error("error: unknown operation");
  return nullptr;
} 
llvm::Value *Codegen::op_div(llvm::Value *lhs, llvm::Value *rhs) {
  IMPLICIT_CAST;
  if(lhs->getType()->isIntegerTy() && rhs->getType()->isIntegerTy()) {
    return builder.CreateSDiv(lhs, type_cast(rhs, lhs->getType()));
  } else if(lhs->getType()->isDoubleTy()) {
    return builder.CreateFDiv(lhs, rhs);
  } else error("error: unknown operation");
  return nullptr;
} 
llvm::Value *Codegen::op_rem(llvm::Value *lhs, llvm::Value *rhs) {
  if(lhs->getType()->isIntegerTy() && rhs->getType()->isIntegerTy()) {
    return builder.CreateSRem(lhs, type_cast(rhs, lhs->getType()));
  } else error("error: unknown operation");
  return nullptr;
} 
llvm::Value *Codegen::op_and(llvm::Value *lhs, llvm::Value *rhs) {
  if(lhs->getType()->isIntegerTy() && rhs->getType()->isIntegerTy()) {
    return builder.CreateAnd(lhs, type_cast(rhs, lhs->getType()));
  } else error("error: unknown operation");
  return nullptr;
} 
llvm::Value *Codegen::op_or(llvm::Value *lhs, llvm::Value *rhs) {
  if(lhs->getType()->isIntegerTy() && rhs->getType()->isIntegerTy()) {
    return builder.CreateOr(lhs, type_cast(rhs, lhs->getType()));
  } else error("error: unknown operation");
  return nullptr;
} 
llvm::Value *Codegen::op_xor(llvm::Value *lhs, llvm::Value *rhs) {
  if(lhs->getType()->isIntegerTy() && rhs->getType()->isIntegerTy()) {
    return builder.CreateXor(lhs, type_cast(rhs, lhs->getType()));
  } else error("error: unknown operation");
  return nullptr;
} 
llvm::Value *Codegen::op_eq(llvm::Value *lhs, llvm::Value *rhs) {
  IMPLICIT_CAST;
  if(lhs->getType()->isIntegerTy() && rhs->getType()->isIntegerTy()) {
    return builder.CreateICmpEQ(lhs, type_cast(rhs, lhs->getType()));
  } else if(lhs->getType()->isDoubleTy()) {
    return builder.CreateFCmpOEQ(lhs, rhs);
  } else error("error: unknown operation");
  return nullptr;
} 
llvm::Value *Codegen::op_ne(llvm::Value *lhs, llvm::Value *rhs) {
  IMPLICIT_CAST;
  if(lhs->getType()->isIntegerTy() && rhs->getType()->isIntegerTy()) {
    return builder.CreateICmpNE(lhs, type_cast(rhs, lhs->getType()));
  } else if(lhs->getType()->isDoubleTy()) {
    return builder.CreateFCmpONE(lhs, rhs);
  } else error("error: unknown operation");
  return nullptr;
} 
llvm::Value *Codegen::op_lt(llvm::Value *lhs, llvm::Value *rhs) {
  IMPLICIT_CAST;
  if(lhs->getType()->isIntegerTy() && rhs->getType()->isIntegerTy()) {
    return builder.CreateICmpSLT(lhs, type_cast(rhs, lhs->getType()));
  } else if(lhs->getType()->isDoubleTy()) {
    return builder.CreateFCmpOLT(lhs, rhs);
  } else error("error: unknown operation");
  return nullptr;
} 
llvm::Value *Codegen::op_gt(llvm::Value *lhs, llvm::Value *rhs) {
  IMPLICIT_CAST;
  if(lhs->getType()->isIntegerTy() && rhs->getType()->isIntegerTy()) {
    return builder.CreateICmpSGT(lhs, type_cast(rhs, lhs->getType()));
  } else if(lhs->getType()->isDoubleTy()) {
    return builder.CreateFCmpOGT(lhs, rhs);
  } else error("error: unknown operation");
  return nullptr;
} 
llvm::Value *Codegen::op_le(llvm::Value *lhs, llvm::Value *rhs) {
  IMPLICIT_CAST;
  if(lhs->getType()->isIntegerTy() && rhs->getType()->isIntegerTy()) {
    return builder.CreateICmpSLE(lhs, type_cast(rhs, lhs->getType()));
  } else if(lhs->getType()->isDoubleTy()) {
    return builder.CreateFCmpOLE(lhs, rhs);
  } else error("error: unknown operation");
  return nullptr;
} 
llvm::Value *Codegen::op_ge(llvm::Value *lhs, llvm::Value *rhs) {
  IMPLICIT_CAST;
  if(lhs->getType()->isIntegerTy() && rhs->getType()->isIntegerTy()) {
    return builder.CreateICmpSGE(lhs, type_cast(rhs, lhs->getType()));
  } else if(lhs->getType()->isDoubleTy()) {
    return builder.CreateFCmpOGE(lhs, rhs);
  } else error("error: unknown operation");
  return nullptr;
} 

#undef IMPLICIT_CAST

llvm::Value *Codegen::statement(ArrayAST *st) {
  return create_const_array(st->elems);
}

llvm::Value *Codegen::statement(TypeCastAST *st) {
  return type_cast(statement(st->expr), st->cast_to);
}

llvm::Value *Codegen::statement(UnaryAST *st) {
  if(st->op == "&") { // address
    return get_value(st->expr);
  } else if(st->op == "*") {
    auto e = statement(st->expr);
    return builder.CreateLoad(e);
  } else if(st->op == "-") {
    return op_sub(make_int(0), statement(st->expr));
  } else if(st->op == "++") {
    auto v1 = get_value(st->expr);
    auto v  = builder.CreateLoad(v1);
    auto vv = op_add(v, make_int(1));
    asgmt_value(v1, vv);
    return st->postfix ? v : vv;
  } else if(st->op == "--") {
    auto v1 = get_value(st->expr);
    auto v  = builder.CreateLoad(v1);
    auto vv = op_sub(v, make_int(1));
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
    ret = op_add(lhs, rhs);
  } else if(st->op == "-") {
    ret = op_sub(lhs, rhs);
  } else if(st->op == "*") {
    ret = op_mul(lhs, rhs);
  } else if(st->op == "/") {
    ret = op_div(lhs, rhs);
  } else if(st->op == "%") {
    ret = op_rem(lhs, rhs);
  } else if(st->op == "==") {
    ret = op_eq(lhs, rhs);
  } else if(st->op == "!=") {
    ret = op_ne(lhs, rhs);
  } else if(st->op == "<=") {
    ret = op_le(lhs, rhs);
  } else if(st->op == (">=")) {
    ret = op_ge(lhs, rhs);
  } else if(st->op == "<") {
    ret = op_lt(lhs, rhs);
  } else if(st->op == (">")) {
    ret = op_gt(lhs, rhs);
  } else if(st->op == "&" || st->op == "&&") {
    ret = op_and(lhs, rhs);
  } else if(st->op == "|" || st->op == "||") {
    ret = op_or(lhs, rhs);
  } else if(st->op == "^") {
    ret = op_xor(lhs, rhs);
  }
  return ret;
}

llvm::Value *Codegen::statement(TernaryAST *st) {
  llvm::Value *val_cond = statement(st->cond);
  val_cond = builder.CreateICmpNE(
      val_cond, 
      val_cond->getType()->isPointerTy() ?
      llvm::ConstantPointerNull::getNullValue(val_cond->getType()) :
      make_int(0, val_cond->getType()));

  auto *func = builder.GetInsertBlock()->getParent();

  llvm::BasicBlock *bb_then = llvm::BasicBlock::Create(context, "then", func);
  llvm::BasicBlock *bb_else = llvm::BasicBlock::Create(context, "else", func);
  llvm::BasicBlock *bb_merge= llvm::BasicBlock::Create(context, "merge",func);

  builder.CreateCondBr(val_cond, bb_then, bb_else);
  builder.SetInsertPoint(bb_then);

  auto val_then = statement(st->then_expr);
  builder.CreateBr(bb_merge);
  bb_then = builder.GetInsertBlock();

  builder.SetInsertPoint(bb_else);

  auto val_else = statement(st->else_expr);
  builder.CreateBr(bb_merge);
  bb_else = builder.GetInsertBlock();

  builder.SetInsertPoint(bb_merge);

  llvm::PHINode *pnode = builder.CreatePHI(val_then->getType(), 2);
  pnode->addIncoming(val_then, bb_then);
  pnode->addIncoming(val_else, bb_else);
  return pnode;
}

llvm::Value *Codegen::statement(DotOpAST *st) {
  return builder.CreateLoad(get_value(st));
}

llvm::Value *Codegen::statement(SizeofAST *st) {
  return make_int(data_layout->getTypeAllocSize(st->type));
}

llvm::Value *Codegen::statement(StringAST *st) {
  return builder.CreateGlobalStringPtr(st->str);
}

llvm::Value *Codegen::statement(NumberAST *st) {
  if(st->is_float) {
    return llvm::ConstantFP::get(builder.getDoubleTy(), st->f_number);
  } else 
    return make_int(st->i_number);
}

var_t *Codegen::lookup_var(std::string name) {
  var_t *v = nullptr;
  if(cur_func) v = cur_func->block_list.lookup_var(name);
  if(!v) return global_var.get(name);
  return v;
}
