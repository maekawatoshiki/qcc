./qcc $1.c -o $1.bc > /dev/null
llc-3.6 $1.bc -o $1.s
clang $1.s -D"TEST_NAME=\"$1\"" test/main.c -o $1.bin
