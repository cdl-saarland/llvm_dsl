#!/bin/bash

SCRIPT_PATH=$(dirname $(realpath -s $0))

mkdir -p $SCRIPT_PATH/build && cd $SCRIPT_PATH/build
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
ninja

# generates kernel.ll
./YourDSL

clang++ -O3 ../main.cpp kernel.ll -o run

./run 15 35 10 > YourDSL.out
./run 2 2 1 >> YourDSL.out
./run 100 12 35 >> YourDSL.out

# generates kernelsol.ll
./YourDSLSol

clang++ -O3 ../solution/main.cpp kernelsol.ll -o runSol

./runSol 15 35 10 > YourDSLSol.out
./runSol 2 2 1 >> YourDSLSol.out
./runSol 100 12 35 >> YourDSLSol.out

if [[ $(cmp YourDSL.out YourDSLSol.out) ]]; then
  echo "YourDSL.out and YourDSLSol.out differ"
  exit 1
fi
echo "YourDSL.out and YourDSLSol.out are the same"
