#include "ast.hpp"

FunctionProtoAST::FunctionProtoAST(std::string _name, llvm::FunctionType *fty, int _stg):
  stg(_stg), name(_name), func_type(fty) {
}

FunctionDefAST::FunctionDefAST(std::string _name, llvm::FunctionType *fty, std::vector<std::string> _args, AST_vec _body, int _stg):
  stg(_stg), name(_name), func_type(fty), args_name(_args), body(_body) {
}

FunctionCallAST::FunctionCallAST(AST *_callee, AST_vec _args):
  callee(_callee), args(_args) {
}

BlockAST::BlockAST(AST_vec _body):
  body(_body) {
}

ArrayAST::ArrayAST(AST_vec _elems):
  elems(_elems) {
}

TypeCastAST::TypeCastAST(AST *_expr, llvm::Type *_cast_to):
  expr(_expr), cast_to(_cast_to) {
}

UnaryAST::UnaryAST(std::string _op, AST *_expr, bool _postfix):
  expr(_expr), op(_op), postfix(_postfix) {
}

BinaryAST::BinaryAST(std::string _op, AST *_lhs, AST *_rhs):
  op(_op), lhs(_lhs), rhs(_rhs) {
}

TernaryAST::TernaryAST(AST *_cond, AST *_then, AST *_else):
  cond(_cond), then_expr(_then), else_expr(_else) {
}

DotOpAST::DotOpAST(AST *_lhs, AST *_rhs, bool _arrow):
  lhs(_lhs), rhs(_rhs), is_arrow(_arrow) {
}

VarDeclarationAST::VarDeclarationAST(std::vector<declarator_t *> _decls, int _stg):
  decls(_decls), stg(_stg) {
}

TypedefAST::TypedefAST(llvm::Type *_from, std::string _to):
  from(_from), to(_to) {
}

VariableAST::VariableAST(std::string _name):
  name(_name) {
}

IndexAST::IndexAST(AST *_ary, AST *_idx):
  ary(_ary), idx(_idx) {
}

IfAST::IfAST(AST *_cond, AST *_then, AST *_else):
  cond(_cond), b_then(_then), b_else(_else) {
}

WhileAST::WhileAST(AST *_cond, AST *_body):
  cond(_cond), body(_body) {
}

ForAST::ForAST(AST *_init, AST *_cond, AST *_reinit, AST *_body):
  init(_init), cond(_cond), reinit(_reinit), body(_body) {
}

AsgmtAST::AsgmtAST(AST *_dst, AST *_src):
  dst(_dst), src(_src) {
}

ReturnAST::ReturnAST(AST *_expr):
  expr(_expr) {
}

SizeofAST::SizeofAST(llvm::Type *_type):
  type(_type) {
}

SizeofAST::SizeofAST(AST *_ast):
  ast(_ast), is_expr(true) {
}

NumberAST::NumberAST(int num):
  i_number(num) {
}

NumberAST::NumberAST(double num):
  is_float(true), f_number(num) {
}

StringAST::StringAST(std::string _str):
  str(_str) {
}

