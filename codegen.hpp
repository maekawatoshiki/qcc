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
    func_t *cur_func;

    llvm::Value *statement(AST *, Type *);
    llvm::Value *statement(FunctionDefAST *    , Type *);
    llvm::Value *statement(FunctionProtoAST *  , Type *);
    llvm::Value *statement(BlockAST *          , Type *);
    llvm::Value *statement(FunctionCallAST *   , Type *);
    llvm::Value *statement(VarDeclarationAST * , Type *);
    llvm::Value *statement(VariableAST *       , Type *);
    llvm::Value *statement(IfAST *             , Type *);
    llvm::Value *statement(WhileAST *          , Type *);
    llvm::Value *statement(ReturnAST *         , Type *);
    llvm::Value *statement(AsgmtAST *          , Type *);
    llvm::Value *statement(BinaryAST *         , Type *);
    llvm::Value *statement(StringAST *         , Type *);
    llvm::Value *statement(NumberAST *         , Type *);
    llvm::Type  *to_llvm_type(Type *);
    llvm::Value *type_cast(llvm::Value *, llvm::Type *);
    llvm::AllocaInst *create_entry_alloca(llvm::Function *TheFunction, std::string &VarName, llvm::Type *type = nullptr);

    void error(const char *errs, ...);
  public:
    void run(AST_vec);    
};
