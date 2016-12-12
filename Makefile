CXX := clang++-3.
LLVM_CONFIG=llvm-config-3.5
CXXFLAGS := -O3 -std=c++11 -MMD -MP $(shell $(LLVM_CONFIG) --cxxflags)
LIBS := -lm $(shell $(LLVM_CONFIG) --system-libs --ldflags --libs all)

PROG := qcc
SRCS := $(wildcard *.cpp)
OBJS := $(SRCS:%.cpp=%.o)
DEPS := $(SRCS:%.cpp=%.d)

TESTS := $(patsubst %.c,%,$(filter-out test/main.c, $(wildcard test/*.c)))

.PHONY: test clean

$(PROG): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ -rdynamic $(OBJS) $(LIBS)

test: $(PROG)
	@for t in $(TESTS); do \
		./test/test.sh $$t; \
	done
	@for t in $(TESTS); do \
		$$t.bin || exit; \
	done

clean:
	-$(RM) $(PROG) $(OBJS) $(DEPS) $(TESTS:%=%.bc) $(TESTS:%=%.s) $(TESTS:%=%.bin)

-include $(DEPS)
