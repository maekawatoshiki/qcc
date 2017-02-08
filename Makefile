CXX := clang++-3.8
LLVM_CONFIG := llvm-config-3.8
CXXFLAGS := -fsanitize=address -MMD -MP $(shell $(LLVM_CONFIG) --cxxflags)
LIBS := -lm $(shell $(LLVM_CONFIG) --system-libs --ldflags --libs all)

PROG := qcc
SRCS := $(wildcard src/*.cpp)
OBJS := $(SRCS:%.cpp=%.o)
DEPS := $(SRCS:%.cpp=%.d)

TESTS := $(patsubst %.c,%,$(filter-out test/main.c, $(wildcard test/*.c)))

.PHONY: test clean

$(PROG): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ -rdynamic $(OBJS) $(LIBS)

test: $(PROG)
	@for t in $$(ls example); do \
		./qcc example/$$t > /dev/null || exit; \
  done
	@for t in $(TESTS); do \
		./test/test.sh $$t; \
  done
	@for t in $(TESTS); do \
		$$t.bin || exit; \
  done

clean:
	-$(RM) $(PROG) $(OBJS) $(DEPS) $(TESTS:%=%.bc) $(TESTS:%=%.s) $(TESTS:%=%.bin)

-include $(DEPS)
