#include "qcc.hpp"

int QCC::run(std::string source) {
  token = LEX.run(source);
  token = PP.run(token);
  token.add_end_tok();
  std::cout << "AFTER LEXICAL ANALYZE:\n"; token.show();
  auto ast = PARSE.run(token);puts("parser process exited successfully");
  CODEGEN.struct_list = PARSE.struct_list;
  CODEGEN.run(ast, out_file_name, emit_llvm_ir);
  return 0;
}

void QCC::set_out_file_name(std::string name) { out_file_name = name; }

void QCC::set_emit_llvm_ir(bool e) { emit_llvm_ir = e; }
