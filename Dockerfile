FROM ubuntu:16.04

RUN \
  apt-get update && \
  apt-get upgrade -y && \
  apt-get install zlib1g-dev -y && \
  apt-get install clang-3.5 clang-3.5-doc libclang-common-3.5-dev libclang-3.5-dev libclang1-3.5 libclang1-3.5-dbg libllvm3.5v5 libllvm3.5-dbg lldb-3.5 llvm-3.5 llvm-3.5-dev llvm-3.5-doc llvm-3.5-examples llvm-3.5-runtime clang-modernize-3.5 clang-format-3.5 python-clang-3.5 lldb-3.5-dev libedit-dev build-essential make -y; \
  ln -s /usr/bin/clang-3.5 /usr/bin/clang; \
  ln -s /usr/bin/clang++-3.5 /usr/bin/clang++;

ADD . /opt/qcc

WORKDIR /opt/qcc

RUN make 
