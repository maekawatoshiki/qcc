./qcc $1 -emit-ir
opt-3.5 -std-compile-opts a.bc -o a.bc
llc-3.5 a.bc -O3
cc a.s -O3 -lm
rm a.s a.bc
