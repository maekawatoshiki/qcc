#include "qcc.hpp"

// WELCOME TO QCC WORLD!!
// QCC IS A SMALL TOY COMPILER FOR C LANGUAGE

void show_usage() {
  puts("qcc - a small toy compiler for C langauge (v" QCC_VERSION ", BUILD " __DATE__ " " __TIME__ ")");
  puts("./qcc [options] file...");
  puts("options:");
  puts("  -o <name>  : place the output into <name>");
  puts("  -emit-ir   : output LLVM-IR to stdout");
  exit(0);
}

int main(int argc, char *argv[]) {
  if(argc < 2) show_usage();

  QCC qcc;

  // TODO: FIXME: Here is a simple option parser.
  std::string ofile, infile; 
  bool emit_llvm_ir = false;
  for(int i = 0; i < argc; i++) {
    if(!strcmp(argv[i], "-o")) {
      ofile = argv[++i]; 
    } else if(!strcmp(argv[i], "-emit-ir")) {
      emit_llvm_ir = true;
    } else if(!strcmp(argv[i], "-h")) {
      show_usage();
    } else infile = argv[i];
  }

  std::string source = [&]() -> std::string {
    std::ifstream ifs_src(infile);
    if(!ifs_src) { printf("file not found '%s'\n", argv[1]); exit(0); }
    std::istreambuf_iterator<char> it(ifs_src), last;
    std::string src_all(it, last);
    return src_all;
  }();

  if(!ofile.empty()) qcc.set_out_file_name(ofile);
  qcc.set_emit_llvm_ir(emit_llvm_ir);
  qcc.run(source);
}
