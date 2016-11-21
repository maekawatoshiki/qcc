./qcc $1 -emit-ir
llc-3.5 a.bc -O3
opt-3.5 a.bc -o a.bc
clang a.s -O3
#rm a.s a.bc
