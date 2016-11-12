./qcc $1 -emit-ir
llc-3.5 a.bc -O3
clang a.s -O3
rm a.s a.bc
