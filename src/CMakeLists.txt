cmake_minimum_required(VERSION 2.8)
project(qcc)

add_executable(qcc 
  ast.cpp
  codegen.cpp
  expr.cpp
  func.cpp
  lexer.cpp
  main.cpp
  parse.cpp
  qcc.cpp
  struct.cpp
  token.cpp
  type.cpp
  var.cpp
)
target_link_libraries(qcc 
  ${LLVM_LIBS_ALL} 
  ${LIBGC_LIBRARIES})
