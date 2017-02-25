#include "ast.hpp"

FunctionProtoAST::FunctionProtoAST(std::string _name, llvm::FunctionType *fty, int _stg):
  stg(_stg), name(_name), func_type(fty) {
}
void FunctionProtoAST::show() {
  std::cerr << "(func-proto ";
  std::cerr << debug_ast::llvmty_to_str(this->func_type) << " " << this->name << ")" << std::endl;
}

FunctionDefAST::FunctionDefAST(std::string _name, llvm::FunctionType *fty, std::vector<std::string> _args, AST_vec _body, int _stg):
  stg(_stg), name(_name), func_type(fty), args_name(_args), body(_body) {
}
void FunctionDefAST::show() {
  std::cerr << "(func-def ";
  std::cerr << debug_ast::llvmty_to_str(this->func_type) << " " << this->name << " (" << std::endl;
  for(auto b : this->body) {
    b->show();
  }
  std::cerr << "))" << std::endl;
}

FunctionCallAST::FunctionCallAST(AST *_callee, AST_vec _args):
  callee(_callee), args(_args) {
}
void FunctionCallAST::show() {
  std::cerr << "(func-call ";
    this->callee->show();
    std::cerr << " (";
      for(auto arg : this->args) {
        arg->show(); std::cerr << " ";
      }
    std::cerr << ")";
  std::cerr << ")" << std::endl;;
}

BlockAST::BlockAST(AST_vec _body):
  body(_body) {
}
void BlockAST::show() {
  std::cerr << "(" << std::endl;
  for(auto s : this->body)
    s->show();
  std::cerr << ")" << std::endl;
}

ArrayAST::ArrayAST(AST_vec _elems):
  elems(_elems) {
}
void ArrayAST::show() {
  std::cerr << "(array ";
    for(auto e : this->elems) {
      e->show();
    }
  std::cerr << ")" << std::endl;
}

TypeCastAST::TypeCastAST(AST *_expr, llvm::Type *_cast_to):
  expr(_expr), cast_to(_cast_to) {
}
void TypeCastAST::show() {
  std::cerr << "(type-cast ";
    this->expr->show();
    std::cerr << " " << debug_ast::llvmty_to_str(this->cast_to);
  std::cerr << ")" << std::endl;
}

UnaryAST::UnaryAST(std::string _op, AST *_expr, bool _postfix):
  expr(_expr), op(_op), postfix(_postfix) {
}
void UnaryAST::show() {
  std::cerr << "(unary-" << (this->postfix ? "postfix " : "prefix ") << this->op << " ";
  this->expr->show();
  std::cerr << ")" << std::endl;
}

BinaryAST::BinaryAST(std::string _op, AST *_lhs, AST *_rhs):
  op(_op), lhs(_lhs), rhs(_rhs) {
}
void BinaryAST::show() {
  std::cerr << "(" << this->op << " ";
    this->lhs->show(); std::cerr << " ";
    this->rhs->show();
  std::cerr << ")" << std::endl;
}

TernaryAST::TernaryAST(AST *_cond, AST *_then, AST *_else):
  cond(_cond), then_expr(_then), else_expr(_else) {
}
void TernaryAST::show() {
  std::cerr << "(ternary (";
  this->cond->show(); std::cerr << ") (";
  this->then_expr->show(); std::cerr << ") (";
  if(this->else_expr) this->else_expr->show(); 
  std::cerr << "))" << std::endl;
}

DotOpAST::DotOpAST(AST *_lhs, AST *_rhs, bool _arrow):
  lhs(_lhs), rhs(_rhs), is_arrow(_arrow) {
}
void DotOpAST::show() {
  std::cerr << "(dot ";
    this->lhs->show(); std::cerr << " ";
    this->rhs->show(); 
  std::cerr << ")" << std::endl;
}

VarDeclarationAST::VarDeclarationAST(std::vector<declarator_t *> _decls, int _stg):
  decls(_decls), stg(_stg) {
}
void VarDeclarationAST::show() {
  std::cerr << "(var-decl ";
    for(auto v : this->decls) {
      std::cerr << "(" << debug_ast::llvmty_to_str(v->type) << " " << v->name << " ";
      if(v->init_expr) v->init_expr->show();
      std::cerr << ") ";
    }
  std::cerr << ")" << std::endl;
}

TypedefAST::TypedefAST(llvm::Type *_from, std::string _to):
  from(_from), to(_to) {
}
void TypedefAST::show() {
  std::cerr << "(typedef " << 
    debug_ast::llvmty_to_str(this->from) << " " << this->to <<
    ")" << std::endl;
}

VariableAST::VariableAST(std::string _name):
  name(_name) {
}
void VariableAST::show() {
  std::cerr << "(var " << this->name << ")" << std::endl;
}

void BreakAST::show() {
  std::cerr << "(break)" << std::endl;
}
void ContinueAST::show() {
  std::cerr << "(continue)" << std::endl;
}

IndexAST::IndexAST(AST *_ary, AST *_idx):
  ary(_ary), idx(_idx) {
}
void IndexAST::show() {
  std::cerr << "(index ";
    this->ary->show(); std::cerr << " ";
    this->idx->show();
  std::cerr << ")" << std::endl;
}

IfAST::IfAST(AST *_cond, AST *_then, AST *_else):
  cond(_cond), b_then(_then), b_else(_else) {
}
void IfAST::show() {
  std::cerr << "(if (";
    this->cond->show(); std::cerr << ") (";
    this->b_then->show(); std::cerr << ") (";
    if(this->b_else) this->b_else->show(); 
  std::cerr << "))" << std::endl;
}

WhileAST::WhileAST(AST *_cond, AST *_body):
  cond(_cond), body(_body) {
}
void WhileAST::show() {
  std::cerr << "(while (";
    this->cond->show(); std::cerr << ") (";
    this->body->show(); std::cerr << "))" << std::endl;
}

ForAST::ForAST(AST *_init, AST *_cond, AST *_reinit, AST *_body):
  init(_init), cond(_cond), reinit(_reinit), body(_body) {
}
void ForAST::show() {
  std::cerr << "(for (";
    this->init->show(); std::cerr << ") (";
    this->cond->show(); std::cerr << ") (";
    this->reinit->show(); std::cerr << ") (";
    this->body->show(); std::cerr << "))" << std::endl;
}

AsgmtAST::AsgmtAST(AST *_dst, AST *_src):
  dst(_dst), src(_src) {
}
void AsgmtAST::show() {
  std::cerr << "(assign ";
    this->dst->show(); std::cerr << " ";
    this->src->show(); 
  std::cerr << ")" << std::endl;
}

ReturnAST::ReturnAST(AST *_expr):
  expr(_expr) {
}
void ReturnAST::show() {
  std::cerr << "(return ";
  this->expr->show();
  std::cerr << ")" << std::endl;
}

GotoAST::GotoAST(std::string _label_name):
  label_name(_label_name) {
}
void GotoAST::show() {
  std::cerr << "(goto " << this->label_name << ")" << std::endl;
}

LabelAST::LabelAST(std::string _name):
  name(_name) {
}
void LabelAST::show() {
  std::cerr << "(label " << this->name << ")" << std::endl;
}

SizeofAST::SizeofAST(AST *_expr):
  expr(_expr) {
}
void SizeofAST::show() {
  std::cerr << "(sizeof ";
  this->expr->show();
  std::cerr << ")" << std::endl;
}

NumberAST::NumberAST(int num):
  i_number(num) {
}

NumberAST::NumberAST(double num):
  is_float(true), f_number(num) {
}
void NumberAST::show() {
  if(this->is_float)
    std::cerr << this->f_number;
  else
    std::cerr << this->i_number;
}

StringAST::StringAST(std::string _str):
  str(_str) {
}
void StringAST::show() {
  std::cerr << "\"" << this->str << "\"";
}

namespace debug_ast { 
  void show(AST_vec ast) {
    for(auto a : ast)
      a->show();
  }
  void show(AST *ast) {
    ast->show();
  }
  std::string llvmty_to_str(llvm::Type *ty) {
    std::string str; llvm::raw_string_ostream r(str);
    ty->print(r);
    return r.str();
  }
};
