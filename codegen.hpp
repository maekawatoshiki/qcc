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
    FunctionList func_list;
    std::map<std::string, llvm::Type *> typedef_map;
    VariableList global_var;
    func_t *cur_func = nullptr;

    llvm::Function *tool_memcpy;

    llvm::Value *op_add(llvm::Value *, llvm::Value *);
    llvm::Value *op_sub(llvm::Value *, llvm::Value *);
    // llvm::Value *op_eq (llvm::Value *, llvm::Value *);

    llvm::Value *statement(AST *                 ); 
    llvm::Value *statement(FunctionDefAST *      ); 
    llvm::Value *statement(FunctionProtoAST *    ); 
    llvm::Value *statement(BlockAST *            ); 
    llvm::Value *statement(FunctionCallAST *     ); 
    llvm::Value *statement(VarDeclarationAST *   ); 
    llvm::Type  *statement(TypedefAST *          ); 
    llvm::Value *statement(VariableAST *         ); 
    llvm::Value *statement(IndexAST *            ); 
    llvm::Value *statement(BreakAST *            ); 
    llvm::Value *statement(ContinueAST *         ); 
    llvm::Value *statement(IfAST *               ); 
    llvm::Value *statement(WhileAST *            ); 
    llvm::Value *statement(ForAST *              ); 
    llvm::Value *statement(ReturnAST *           ); 
    llvm::Value *statement(AsgmtAST *            ); 
    llvm::Value *statement(ArrayAST *            ); 
    llvm::Value *statement(UnaryAST *            ); 
    llvm::Value *statement(BinaryAST *           ); 
    llvm::Value *statement(DotOpAST *            ); 
    llvm::Value *statement(StringAST *           ); 
    llvm::Value *statement(NumberAST *           ); 
    llvm::Value *get_element_ptr(IndexAST *      ); 
    llvm::Value *get_value(AST *                 ); 
    llvm::Value *asgmt_value(llvm::Value *, llvm::Value *src);
    llvm::Value *type_cast(llvm::Value *, llvm::Type *);
    llvm::AllocaInst *create_entry_alloca(llvm::Function *TheFunction, std::string &VarName, llvm::Type *type = nullptr);
    var_t *lookup_var(std::string);
  public:
    StructList struct_list;
    void run(AST_vec, std::string = "a.bc", bool emit_llvm_ir = false);    
};
