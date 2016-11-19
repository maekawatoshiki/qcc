#include "ast.hpp"

FunctionProtoAST::FunctionProtoAST(std::string _name, llvm::Type *ret, Type_vec _args):
  name(_name), ret_type(ret), args(_args) {
}

FunctionDefAST::FunctionDefAST(std::string _name, llvm::Type *ret, std::vector<argument_t *> _args, AST_vec _body):
  name(_name), ret_type(ret), args(_args), body(_body) {
}

FunctionCallAST::FunctionCallAST(std::string _name, AST_vec _args):
  name(_name), args(_args) {
}

BlockAST::BlockAST(AST_vec _body):
  body(_body) {
}

ArrayAST::ArrayAST(AST_vec _elems):
  elems(_elems) {
}

UnaryAST::UnaryAST(std::string _op, AST *_expr, bool _postfix):
  op(_op), expr(_expr), postfix(_postfix) {
}

BinaryAST::BinaryAST(std::string _op, AST *_lhs, AST *_rhs):
  op(_op), lhs(_lhs), rhs(_rhs) {
}

DotOpAST::DotOpAST(AST *_lhs, AST *_rhs, bool _arrow):
  lhs(_lhs), rhs(_rhs), is_arrow(_arrow) {
}

VarDeclarationAST::VarDeclarationAST(std::vector<declarator_t *> _decls):
  decls(_decls) {
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

NumberAST::NumberAST(int num):
  number(num) {
}

StringAST::StringAST(std::string _str):
  str(_str) {
}

