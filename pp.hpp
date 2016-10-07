#pragma once

#include "common.hpp"

class Preprocessor {
  private:
    std::string default_include_path = "./include/";
  public:
    std::string run(std::string);
};
