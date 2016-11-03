#pragma once

#include "common.hpp"
#include "ast.hpp"
#include "func.hpp"
#include "struct.hpp"

extern llvm::LLVMContext &context;
extern llvm::IRBuilder<> builder;
extern llvm::Module *mod;
class Codegen {
  private:
    StructList struct_list;
    FunctionList func_list;
    std::map<std::string, llvm::Type *> typedef_map;
    func_t *cur_func;

    llvm::Value *statement(AST *                 ); 
    llvm::Value *statement(FunctionDefAST *      ); 
    llvm::Value *statement(FunctionProtoAST *    ); 
    llvm::Value *statement(BlockAST *            ); 
    llvm::Value *statement(FunctionCallAST *     ); 
    llvm::Value *statement(VarDeclarationAST *   ); 
    llvm::Type  *statement(TypedefAST *          ); 
    llvm::Type  *statement(StructDeclarationAST *); 
    llvm::Value *statement(VariableAST *         ); 
    llvm::Value *statement(IndexAST *            ); 
    llvm::Value *statement(IfAST *               ); 
    llvm::Value *statement(WhileAST *            ); 
    llvm::Value *statement(ForAST *              ); 
    llvm::Value *statement(ReturnAST *           ); 
    llvm::Value *statement(AsgmtAST *            ); 
    llvm::Value *statement(UnaryAST *            ); 
    llvm::Value *statement(BinaryAST *           ); 
    llvm::Value *statement(DotOpAST *            ); 
    llvm::Value *statement(StringAST *           ); 
    llvm::Value *statement(NumberAST *           ); 
    llvm::Value *get_element_ptr(IndexAST *      ); 
    llvm::Value *get_value(AST *                 ); 
    llvm::Value *asgmt_value(llvm::Value *, llvm::Value *src);
    llvm::Type  *to_llvm_type(Type *);
    llvm::Value *type_cast(llvm::Value *, llvm::Type *);
    llvm::AllocaInst *create_entry_alloca(llvm::Function *TheFunction, std::string &VarName, llvm::Type *type = nullptr);

    void error(const char *errs, ...);
  public:
    void run(AST_vec, std::string = "a.bc", bool emit_llvm_ir = false);    
};
