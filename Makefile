CXXFLAGS = -O3
LIBFLAGS = -lm
CXX = clang++ $(CXXFLAGS)

qcc: main.o qcc.o
	$(CXX) -o qcc main.o $(LIBFLAGS)

main.o: main.cpp common.hpp

qcc.o: qcc.cpp qcc.hpp common.hpp

clean:
	$(RM) qcc *.o
