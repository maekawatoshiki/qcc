#pragma once

#include "common.hpp"
#include "ast.hpp"
#include "func.hpp"
#include "struct.hpp"

extern llvm::IRBuilder<> builder;
extern llvm::Module *mod;
extern llvm::DataLayout *data_layout;

class Codegen {
  private:
    FunctionList func_list;
    std::map<std::string, llvm::Type *> typedef_map;
    VariableList global_var;
    func_t *cur_func = nullptr;
    std::map<std::string, llvm::BasicBlock *> label_map;

    llvm::Function *tool_memcpy;

    llvm::Value *op_add(llvm::Value *, llvm::Value *);
    llvm::Value *op_sub(llvm::Value *, llvm::Value *);
    llvm::Value *op_mul(llvm::Value *, llvm::Value *);
    llvm::Value *op_div(llvm::Value *, llvm::Value *);
    llvm::Value *op_rem(llvm::Value *, llvm::Value *);
    llvm::Value *op_and(llvm::Value *, llvm::Value *);
    llvm::Value *op_or (llvm::Value *, llvm::Value *);
    llvm::Value *op_xor(llvm::Value *, llvm::Value *);
    llvm::Value *op_shl(llvm::Value *, llvm::Value *);
    llvm::Value *op_shr(llvm::Value *, llvm::Value *);
    llvm::Value *op_eq (llvm::Value *, llvm::Value *);
    llvm::Value *op_ne (llvm::Value *, llvm::Value *);
    llvm::Value *op_lt (llvm::Value *, llvm::Value *);
    llvm::Value *op_gt (llvm::Value *, llvm::Value *);
    llvm::Value *op_le (llvm::Value *, llvm::Value *);
    llvm::Value *op_ge (llvm::Value *, llvm::Value *);
    llvm::Value *op_land(AST *, AST *);
    llvm::Value *op_lor (AST *, AST *);

    llvm::Value *make_int(int, llvm::Type * = builder.getInt32Ty());

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
    llvm::Value *statement(GotoAST *             );
    llvm::Value *statement(LabelAST *            );
    llvm::Value *statement(AsgmtAST *            ); 
    llvm::Value *statement(ArrayAST *            ); 
    llvm::Value *statement(TypeCastAST *         ); 
    llvm::Value *statement(UnaryAST *            ); 
    llvm::Value *statement(BinaryAST *           ); 
    llvm::Value *statement(TernaryAST *          ); 
    llvm::Value *statement(DotOpAST *            ); 
    llvm::Value *statement(StringAST *           ); 
    llvm::Value *statement(NumberAST *           ); 
    llvm::Value *statement(SizeofAST *           );

    llvm::Type *get_base_type(llvm::Type *);
    llvm::Constant *constinit_global_var(llvm::GlobalVariable *gv, AST *init_expr);
    llvm::ConstantStruct *to_rectype_initializer(AST *ary, llvm::StructType *);
    llvm::Value *get_value_struct(llvm::Value *, struct_t *, std::string);
    llvm::Value *get_value_union(llvm::Value *, union_t *, std::string);
    void create_var(var_t v, llvm::Value * = nullptr);
    void create_global_var(var_t v, int /*storage ty*/, AST * = nullptr);
    llvm::Constant *create_const_array(std::vector<llvm::Constant *>, int = 0);
    llvm::Value *get_element_ptr(IndexAST *      ); 
    llvm::Value *get_value(AST *                 ); 
    llvm::Value *asgmt_value(llvm::Value *, llvm::Value *src);
    llvm::Value *type_cast(llvm::Value *, llvm::Type *);
    llvm::AllocaInst *create_entry_alloca(llvm::Function *TheFunction, std::string &VarName, llvm::Type *type = nullptr);
    var_t *lookup_var(std::string);
  public:
    StructList struct_list;
    UnionList   union_list;
    void run(AST_vec, std::string = "a.bc", bool emit_llvm_ir = false);    
};
