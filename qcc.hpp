#pragma once

#include "common.hpp"
#include "pp.hpp"

class QCC {
  private:
    Preprocessor PP;
  public:
    int run(std::string);
};
