./qcc $1
llc-3.5 a.bc
clang a.s
rm a.s a.bc
