CXXFLAGS = -O3 -std=c++11
LIBFLAGS = -lm
CXX = clang++ $(CXXFLAGS)

qcc: main.o qcc.o pp.o
	$(CXX) -o qcc main.o qcc.o pp.o $(LIBFLAGS)

main.o: main.cpp common.hpp

qcc.o: qcc.cpp qcc.hpp common.hpp

pp.o: pp.cpp pp.hpp common.hpp

clean:
	$(RM) qcc *.o
