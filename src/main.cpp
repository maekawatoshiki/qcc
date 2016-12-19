#include "qcc.hpp"

// WELCOME TO QCC WORLD!!
// QCC IS A SMALL TOY COMPILER FOR C LANGUAGE

int main(int argc, char *argv[]) {
  if(argc < 2) show_usage();

  QCC qcc(argc, argv);
  qcc.run();
}
