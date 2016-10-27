# QCC
[![Build Status](https://travis-ci.org/maekawatoshiki/qcc.svg?branch=master)](https://travis.org/maekawatoshiki/qcc)
- qcc is a small toy compiler for C language.
- waiting for pull-req!

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
