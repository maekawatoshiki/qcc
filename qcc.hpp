#pragma once

#include "common.hpp"
#include "pp.hpp"
#include "lexer.hpp"
#include "parse.hpp"
#include "codegen.hpp"

class QCC {
  private:
    Preprocessor PP;
    Token token;
    Lexer LEX;
    Parser PARSE;
    Codegen CODEGEN;
    std::string out_file_name = "a.bc";
    bool emit_llvm_ir = false;
  public:
    void set_out_file_name(std::string);
    void set_emit_llvm_ir(bool);
    int run(std::string);
};
