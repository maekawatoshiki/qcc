#pragma once

#include "common.hpp"
#include "lexer.hpp"
#include "parse.hpp"
#include "codegen.hpp"

#define QCC_VERSION "0.3"

class QCC {
  private:
    Token token;
    Lexer LEX;
    Parser PARSE;
    Codegen CODEGEN;

    std::string out_file_name = "a.bc";
    bool emit_llvm_ir = false;

  public:
    int argc;
    char **argv;
    QCC(int _argc, char **_argv):
      argc(_argc), argv(_argv) {};

    void set_out_file_name(std::string);
    void set_emit_llvm_ir(bool);

    void show_usage();
    void show_version();

    int run(); // run following argc and argv
    int run(std::string);
};

