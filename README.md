# QCC
[![Build Status](https://travis-ci.org/maekawatoshiki/qcc.svg?branch=master)](https://travis.org/maekawatoshiki/qcc)
- qcc is a small toy compiler for C language.
- waiting for pull-req!

# FEATURES
- qcc makes bitcode from C source code with LLVM. so we have to convert bitcode to native.
- the shell script './qcc.sh' does it. or do yourself such as below:
```
$ ./qcc c.c -o c.bc # qcc makes c.bc
$ llc-3.5 c.bc # c.bc -> c.s
$ clang c.s # c.s -> a.out
```

# BUILD
- used tools: clang, llvm-3.5
- if you are using Ubuntu or Debian, maybe you can see what you have to apt-get install if you look at Dockerfile.
```
$ make
```

# TRY ON DOCKER
- follow below
```
docker build -t="qcc" .
docker run -it "qcc" .
```
