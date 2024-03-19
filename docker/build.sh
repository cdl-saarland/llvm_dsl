set -xe

docker build \
    -t fodinabor/llvm_workshop:dsl \
    --network=host .
