#include "qcc.hpp"

int QCC::run(std::string source) {
  source = PP.run(source);
  std::cout << "after preprocess:\n" << source << std::endl;
  return 0;
}

