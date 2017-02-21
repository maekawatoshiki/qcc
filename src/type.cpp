#include "type.hpp"

Type::Type(TypeKind _kind, bool _signed): // for integer
  kind(_kind), t_signed(_signed) {
    switch(_kind) {
      case KIND_VOID:      llvm_type = llvm::Type::getVoidTy(context);  break;
      case KIND_CHAR:      llvm_type = llvm::Type::getInt8Ty(context);  break;
      case KIND_SHORT:     llvm_type = llvm::Type::getInt16Ty(context); break;
      case KIND_INT:       llvm_type = llvm::Type::getInt32Ty(context); break;
      case KIND_LONG:      llvm_type = llvm::Type::getInt32Ty(context); break;
      case KIND_LONG_LONG: llvm_type = llvm::Type::getInt64Ty(context); break;
  }
}

Type::Type(TypeKind _kind, int _size, Type *_elem): // for array
  kind(_kind), size(_size), elem_type(_elem) {
  assert(_kind == KIND_ARRAY);
}

Type::Type(TypeKind _kind, Type *_elem): // for ptr
  kind(_kind), elem_type(_elem) {
  assert(_kind == KIND_PTR);
}

Type::Type(TypeKind _kind, std::string _name, llvm::Type *llvm_ty): // for struct union enum
  kind(_kind), name(_name), llvm_type(llvm_ty) {
}

Type::Type(TypeKind _kind, llvm::Type *_llvmty): // for func ptr
  kind(_kind), llvm_type(_llvmty) {
}

Type *Type::get_pointer_to() {
  return new Type(KIND_PTR, this);
}

Type *Type::get() {
  return this;
}

llvm::Type *Type::to_llvm_type() {
  switch(this->kind) {
    case KIND_VOID:      return llvm::Type::getVoidTy(context);  
    case KIND_CHAR:      return llvm::Type::getInt8Ty(context);  
    case KIND_SHORT:     return llvm::Type::getInt16Ty(context); 
    case KIND_INT:       return llvm::Type::getInt32Ty(context); 
    case KIND_LONG:      return llvm::Type::getInt32Ty(context); 
    case KIND_LONG_LONG: return llvm::Type::getInt64Ty(context);
    case KIND_ARRAY:     return llvm::ArrayType::get(this->elem_type->to_llvm_type(), size);
    case KIND_PTR:       return this->elem_type->to_llvm_type()->getPointerTo();
    case KIND_STRUCT:   
    case KIND_UNION:    
    case KIND_ENUM:     
    case KIND_FUNC_PTR:  return this->llvm_type;
  }
}
