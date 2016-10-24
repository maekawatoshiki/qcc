./qcc test/$1.c -o test/$1.bc
llc-3.5 test/$1.bc -o test/$1.s
clang test/$1.s -D"TEST_NAME=\"$1\"" test/main.c -o test/$1.bin
