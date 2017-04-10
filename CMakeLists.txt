project(qcc)
cmake_minimum_required(VERSION 2.8)
set(CMAKE_VERBOSE_MAKEFILE 1)
enable_testing()

find_package(PkgConfig)

set(LLVM_CONFIG "llvm-config-3.8")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})
add_definitions("-O3 -std=c++11")
add_definitions("-D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS") # for LLVM

execute_process(
  COMMAND
  ${LLVM_CONFIG} --includedir
  OUTPUT_VARIABLE
  LLVM_INCLUDES
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(
  COMMAND
  ${LLVM_CONFIG} --libdir
  OUTPUT_VARIABLE
  LLVM_LIBRARIES
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(
  COMMAND
  ${LLVM_CONFIG} --libs all
  OUTPUT_VARIABLE
  LLVM_LIBS_ALL_STRING
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(
  COMMAND
  ${LLVM_CONFIG} --system-libs
  OUTPUT_VARIABLE
  LLVM_SYSTEM_LIBS_STRING
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
set(LLVM_LIBS_ALL)
foreach (l ${LLVM_SYSTEM_LIBS_STRING} ${LLVM_LIBS_ALL_STRING})
    string(REGEX REPLACE "-l([^ ]+)" "-l\\1" lib_name ${l})
    list(APPEND LLVM_LIBS_ALL ${lib_name})
endforeach (l) 

include_directories(${LLVM_INCLUDES} ${libgc_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARIES})
add_subdirectory(src)

# TODO: tentative tests. fix
add_test(8queens.c        "./qcc" "8queens.c")
add_test(ary_fibo.c       "./qcc" "ary_fibo.c")
add_test(ary_ptr.c        "./qcc" "ary_ptr.c")
add_test(brainfuck_fact.c "./qcc" "brainfuck_fact.c")
add_test(calc.c           "./qcc" "calc.c")
add_test(define.c         "./qcc" "define.c")
add_test(define2.c        "./qcc" "define2.c")
add_test(fibo.c           "./qcc" "fibo.c")
add_test(fibo_mat.c       "./qcc" "fibo_mat.c")
add_test(fptr.c           "./qcc" "fptr.c")
add_test(hello.c          "./qcc" "hello.c")
add_test(list.c           "./qcc" "list.c")
add_test(multi_table.c    "./qcc" "multi_table.c")
add_test(pi.c             "./qcc" "pi.c")
add_test(prime.c          "./qcc" "prime.c")
add_test(qsort_sample.c   "./qcc" "qsort_sample.c")
add_test(swap.c           "./qcc" "swap.c")
add_test(time.c           "./qcc" "time.c")
add_test(var-decl.c       "./qcc" "var-decl.c")
add_test(while.c          "./qcc" "while.c")
