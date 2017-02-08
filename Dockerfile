FROM ubuntu:16.04

RUN \
  apt-get update && \
  apt-get upgrade -y && \
  apt-get install zlib1g-dev -y && \
  apt-get install clang-3.6 clang-3.6-doc libclang-common-3.6-dev libclang-3.6-dev libclang1-3.6 libclang1-3.6-dbg libllvm3.6v5 libllvm3.6-dbg lldb-3.6 llvm-3.6 llvm-3.6-dev llvm-3.6-doc llvm-3.6-examples llvm-3.6-runtime clang-modernize-3.6 clang-format-3.6 python-clang-3.6 lldb-3.6-dev opt libedit-dev build-essential make -y; \
  ln -s /usr/bin/clang-3.6 /usr/bin/clang; \
  ln -s /usr/bin/clang++-3.6 /usr/bin/clang++;

ADD . /opt/qcc

WORKDIR /opt/qcc

RUN make 
