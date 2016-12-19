#include "struct.hpp"

void StructList::add(struct_t strct) {
  struct_list.push_back(strct);
}

void StructList::add(std::string name, std::vector<std::string> members_name, llvm::StructType *llvm_strct) {
  struct_list.push_back(
      (struct_t) {
        name,
        members_name,
        llvm_strct
        });
}

struct_t *StructList::get(std::string name) {
  for(auto &s : struct_list) {
    if(s.name == name)
      return &s;
  }
  return nullptr;
}
