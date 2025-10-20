set -xe

docker build \
    -t fodinabor/llvm_workshop:dsl20 \
    --network=host .
