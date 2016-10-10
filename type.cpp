#include "type.hpp"

type_t &Type::get() {
  return this->type;
}

void Type::change(int ty) {
  type.type = ty;
}

void Type::change(std::string ty) {
  type.type = TY_USER;
}

void Type::change(Type *ty) {
  type = ty->type;
  next = ty->next;
}

void Type::change(int ty, Type *ty2) {
  type.type = ty;
  next = ty2;
}

bool Type::is_ptr() {
  if(type.type == TY_PTR) return true;
  return false;
}

bool Type::eql(int ty) {
  return type.type == ty;
}

bool Type::eql(std::string userty) {
  return type.type == TY_USER && type.user_type == userty;
}

bool Type::eql(Type *ty) {
  if(ty->eql(type.type) && ty->eql(type.user_type)) {
    if(next && ty->next) {
      Type *_next = this;
      ty = ty->next;
      return ty->eql(next);
    }
    return true;
  }
  return false;
}


