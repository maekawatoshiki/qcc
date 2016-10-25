./qcc $1 -emit-ir
llc-3.5 a.bc
clang a.s
rm a.s a.bc
