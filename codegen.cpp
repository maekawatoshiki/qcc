#include "codegen.hpp"

llvm::LLVMContext &context(llvm::getGlobalContext());
llvm::IRBuilder<> builder(context);
llvm::Module *mod;

void Codegen::run(AST_vec ast) {
  Type ty;
  mod = new llvm::Module("QCC", context);
  for(auto st : ast) {
    statement(st, &ty);
  }
  mod->dump();
  std::string EC;
  llvm::raw_fd_ostream out("a.bc", EC, llvm::sys::fs::OpenFlags::F_RW);
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
    case TY_PTR:
      return to_llvm_type(type->next)->getPointerTo();
  }
  return nullptr;
}

llvm::Value *Codegen::statement(AST *st, Type *ret_type) {
  switch(st->get_type()) {
    case AST_FUNCTION_DEF:
      return statement((FunctionDefAST *)st, ret_type);
    case AST_FUNCTION_PROTO:
      return statement((FunctionProtoAST *)st, ret_type);
  }
  return nullptr;
}

llvm::Value *Codegen::statement(FunctionProtoAST *st, Type *ret_type) {
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

llvm::Value *Codegen::statement(FunctionDefAST *st, Type *ret_type) {
  func_t func;
  func.name = st->name;
  func.ret_type = st->ret_type;
  std::vector<llvm::Type *> llvm_args_type;
  std::vector<Type *>       args_type;
  std::vector<std::string>  args_name;
  for(auto arg : st->args) {
    func.var_list.add(var_t(arg->name, arg->type));
    func.args_type.push_back(arg->type);
    func.args_name.push_back(arg->name);
    func.llvm_args_type.push_back(to_llvm_type(arg->type));
  }
  this->func_list.add(func);
  func_t *function = this->func_list.get(func.name);

  llvm::FunctionType *llvm_func_type = 
    llvm::FunctionType::get(to_llvm_type(func.ret_type), func.llvm_args_type, false);
  llvm::Function *llvm_func = 
    llvm::Function::Create(llvm_func_type, llvm::Function::ExternalLinkage, func.name, mod);
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

    Type return_type;
    for(auto it : st->body) statement(it, &return_type);
    builder.CreateRet(llvm::ConstantInt::get(builder.getInt32Ty(), 0));
  }

  return nullptr;
}









