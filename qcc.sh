./qcc $1 -emit-ir
opt-3.6 -std-link-opts a.bc -o a.bc
llc-3.6 a.bc -O3
clang a.s -O3 -lm
rm a.s a.bc
