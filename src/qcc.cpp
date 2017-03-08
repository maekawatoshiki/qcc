#include "qcc.hpp"

int QCC::run(std::string source) {
  mod = new llvm::Module("QCC", context);
  data_layout = new llvm::DataLayout(mod);

  // token = LEX.run(source);

  Lexer lex; Token include_tok = lex.run("./include/qcc.h");
  token = LEX.run(source);
  // clock_t b = clock();
  // include_tok = pp.run(include_tok);
  // token = PP.run(token);
  // clock_t e = clock();
  // std::cout << ((double)(e-b)/CLOCKS_PER_SEC) << std::endl;getchar();
  // token.add_end_tok();
  // puts("after preprocess:");
  // token.show(); getchar();
  auto ast = PARSE.run(token); puts("parser process exited successfully");
  CODEGEN.set(PARSE.get_structlist());
  CODEGEN.set(PARSE.get_unionlist());
  CODEGEN.run(ast, out_file_name, emit_llvm_ir);
  return 0;
}

int QCC::run() {
  // TODO: FIXME: Here is a simple option parser.
  std::string ofile, infile; 
  bool emit_llvm_ir = false;
  if(argc < 2) show_usage();
  for(int i = 0; i < argc; i++) {
    if(!strcmp(argv[i], "-o")) {
      ofile = argv[++i]; 
    } else if(!strcmp(argv[i], "-emit-ir")) {
      emit_llvm_ir = true;
    } else if(!strcmp(argv[i], "-h")) {
      show_usage();
    } else if(!strcmp(argv[i], "-v")) {
      show_version(); exit(0);
    } else infile = argv[i];
  }

  // std::string source = [&]() -> std::string {
  //   std::ifstream ifs_src(infile);
  //   if(!ifs_src) { printf("file not found '%s'\n", argv[1]); exit(0); }
  //   std::istreambuf_iterator<char> it(ifs_src), last;
  //   std::string src_all(it, last);
  //   return src_all;
  // }();

  if(!ofile.empty()) set_out_file_name(ofile);
  set_emit_llvm_ir(emit_llvm_ir);
  run(infile);
  return 0;
}

void QCC::set_out_file_name(std::string name) { out_file_name = name; }

void QCC::set_emit_llvm_ir(bool e) { emit_llvm_ir = e; }

void QCC::show_version() {
  puts("qcc - a small toy compiler for C langauge (v" QCC_VERSION ", BUILD " __DATE__ " " __TIME__ ")");
}

void QCC::show_usage() {
  show_version();
  puts("./qcc [options] file...");
  puts("options:");
  puts("  -o <name>  : place the output into <name> (default is 'a.bc')");
  puts("  -emit-ir   : output LLVM-IR to stdout");
  puts("  -h         : show this help");
  puts("  -v         : show version info");
  exit(0);
}

void error(const char *errs, ...) {
  va_list args;
  va_start(args, errs);
    vprintf(errs, args); puts("");
  va_end(args);
  exit(0);
}
