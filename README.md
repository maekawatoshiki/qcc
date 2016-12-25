# QCC
[![Build Status](https://img.shields.io/travis/maekawatoshiki/qcc.svg?style=flat-square)](https://travis-ci.org/maekawatoshiki/qcc)
[![License](http://img.shields.io/badge/license-MIT-blue.svg?style=flat-square)][license]

- qcc is a small toy compiler for C language.
- waiting for pull-req!

# FEATURES
- qcc generates bitcode from C source code with LLVM. so we have to convert bitcode to native.
- the shell script './qcc.sh' does it. or do yourself such as below:
```
$ ./qcc c.c -o c.bc # qcc generates c.bc
$ llc-3.5 c.bc # c.bc -> c.s
$ clang c.s # c.s -> a.out
```

# BUILD
- used tools: clang, llvm-3.5
- if you are using Ubuntu or Debian, maybe you can see what you have to apt-get install if you look at Dockerfile.
```
$ make
$ make test
$ ./qcc.sh <SOURCE CODE>
```

# TRY ON DOCKER
- follow below
```
docker build -t="qcc" .
docker run -it "qcc" .
```
