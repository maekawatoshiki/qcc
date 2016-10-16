#include "type.hpp"
#include "lexer.hpp"

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

std::string Type::to_string() {
  return TypeTool::to_string(this);
}

namespace TypeTool {
  Type *to_type(std::string type_str) {
    Lexer lexer;
    Token tok = lexer.run(type_str);
    std::string type_name = tok.get().val; tok.skip();
    auto ptr = [&](Type *ty) -> Type * {
      while(tok.get().val == "*") {
        ty->next = new Type(ty->get().type);
        ty->change(TY_PTR);
        tok.skip();
      }
      return ty;
    };
    // TODO: support unsigned|signed 
    Type *ty = nullptr;
    if(type_name == "int") {
      ty = new Type(TY_INT);
    } else if(type_name == "char") {
      ty = new Type(TY_CHAR);
    } else if(type_name == "double") {
      ty = new Type(TY_DOUBLE);
    } else {
      ty = new Type(type_name);
    }
    return ptr(ty);
  }
  std::string to_string(Type *type) {
    if(type->eql(TY_PTR)) {
      return to_string(type->next) + "*";
    } else if(type->eql(TY_INT)) {
      return "int";
    } else if(type->eql(TY_CHAR)) {
      return "char";
    } else if(type->eql(TY_DOUBLE)) {
      return "double";
    } else if(type->eql(TY_VARARG)) {
      return "...";
    }
    return "TYPE";
  }
}
