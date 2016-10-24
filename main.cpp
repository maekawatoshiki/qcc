#include "qcc.hpp"

// WELCOME TO QCC WORLD!!
// QCC IS A SMALL TOY COMPILER FOR C LANGUAGE

int main(int argc, char *argv[]) {
  if(argc < 2) {
    puts("usage: qcc <SOURCE FILE>");
    return 0;
  }
  QCC qcc;
  std::string source = [&]() -> std::string {
    std::ifstream ifs_src(argv[1]);
    if(!ifs_src) { puts("file not found"); return ""; }
    std::istreambuf_iterator<char> it(ifs_src), last;
    std::string src_all(it, last);
    return src_all;
  }();
  std::string ofile; // TODO: FIXME!!
  for(int i = 0; i < argc; i++)
    if(!strcmp(argv[i], "-o"))
      ofile = argv[i+1];
  if(!ofile.empty()) qcc.set_out_file_name(ofile);
  qcc.run(source);
}
