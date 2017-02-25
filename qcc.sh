./qcc $1 -emit-ir
opt-3.8 -std-link-opts a.bc -o a.bc
llc-3.8 a.bc -O3
clang-3.8 a.s -O3 -lm
rm a.s a.bc
