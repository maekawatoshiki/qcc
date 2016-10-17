#pragma once

#include "common.hpp"
#include "ast.hpp"
#include "func.hpp"

extern llvm::LLVMContext &context;
extern llvm::IRBuilder<> builder;
extern llvm::Module *mod;
class Codegen {
  private:
    FunctionList func_list;

    llvm::Value *statement(AST *, Type *);
    llvm::Value *statement(FunctionDefAST *, Type *);
    llvm::Value *statement(FunctionProtoAST *, Type *);
    llvm::Type  *to_llvm_type(Type *);
    llvm::AllocaInst *create_entry_alloca(llvm::Function *TheFunction, std::string &VarName, llvm::Type *type = nullptr);
  public:
    void run(AST_vec);    
};
