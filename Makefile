CXXFLAGS = -O3 -std=c++11
LIBFLAGS = -lm
CXX = clang++ $(CXXFLAGS)
LLVM = `llvm-config-3.5 --cppflags `
LLVM_LIB = `llvm-config-3.5 --system-libs --cppflags --ldflags --libs all`
TESTS := $(patsubst %.c,%,$(filter-out test/main.c, $(wildcard test/*.c)))
qcc: main.o qcc.o pp.o token.o lexer.o parse.o ast.o type.o expr.o codegen.o func.o var.o
	$(CXX) -o qcc -rdynamic main.o qcc.o pp.o token.o lexer.o parse.o ast.o \
		type.o expr.o codegen.o func.o var.o $(LIBFLAGS) $(LLVM_LIB)

test: qcc
	@for t in $(TESTS); do \
		./test/test.sh $$t; \
	done
	@for t in $(TESTS); do \
		$$t.bin || exit; \
	done

main.o: main.cpp common.hpp
	$(CXX) -c main.cpp $(LLVM)

qcc.o: qcc.cpp qcc.hpp common.hpp
	$(CXX) -c qcc.cpp $(LLVM)

pp.o: pp.cpp pp.hpp common.hpp
	$(CXX) -c pp.cpp $(LLVM)

token.o: token.cpp token.hpp common.hpp
	$(CXX) -c token.cpp $(LLVM)

lexer.o: lexer.cpp lexer.hpp common.hpp
	$(CXX) -c lexer.cpp $(LLVM)

parse.o: parse.cpp parse.hpp common.hpp
	$(CXX) -c parse.cpp $(LLVM)

ast.o: ast.cpp ast.hpp common.hpp
	$(CXX) -c ast.cpp $(LLVM)

type.o: type.cpp type.hpp common.hpp
	$(CXX) -c type.cpp $(LLVM)

expr.o: expr.cpp expr.hpp common.hpp
	$(CXX) -c expr.cpp $(LLVM)

codegen.o: codegen.cpp codegen.hpp common.hpp
	$(CXX) -c codegen.cpp $(LLVM)

func.o: func.cpp func.hpp common.hpp
	$(CXX) -c func.cpp $(LLVM)

var.o: var.cpp var.hpp common.hpp
	$(CXX) -c var.cpp $(LLVM)

clean:
	$(RM) qcc *.o a.* test/*.bc test/*.s test/*.bin
