FROM silkeh/clang:20 as llvm_dsl

## Global install
RUN apt-get update -y && apt-get install -y \
    git cmake vim bison python3 ninja-build nano less zlib1g-dev libcurl4-openssl-dev gdb lldb lld clangd

# Environment configuration
ENV CC=/usr/bin/clang \
    CXX=/usr/bin/clang++ \
    CMAKE_PREFIX_PATH=/usr/lib/llvm-20/cmake \
    CLANGXX=/usr/bin/clang++ \
    CLANG=/usr/bin/clang \
    OPT=/usr/bin/opt

RUN mkdir -p /opt/bootstrap
COPY entry.sh /opt/bootstrap/entry.sh
RUN chmod a+rx /opt/bootstrap/entry.sh

ENTRYPOINT /opt/bootstrap/entry.sh
