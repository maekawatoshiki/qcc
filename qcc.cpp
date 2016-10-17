#include "qcc.hpp"

int QCC::run(std::string source) {
  source = PP.run(source);
  token = LEX.run(source);
  std::cout << "AFTER PREPROCESS:\n" << source << std::endl;
  std::cout << "AFTER LEXICAL ANALYZE:\n"; token.show();
  auto ast = PARSE.run(token);
  CODEGEN.run(ast);
  return 0;
}

