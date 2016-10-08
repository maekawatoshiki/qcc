#include "qcc.hpp"

int QCC::run(std::string source) {
  source = PP.run(source);
  token = LEX.run(source);
  PARSE.run(token);
  std::cout << "AFTER PREPROCESS:\n" << source << std::endl;
  std::cout << "AFTER LEXICAL ANALYZE:\n"; token.show();
  return 0;
}

