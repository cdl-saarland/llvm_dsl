#!/bin/bash

SCRIPT_PATH=$(dirname $(realpath -s $0))

mkdir -p $SCRIPT_PATH/build && cd $SCRIPT_PATH/build
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
ninja

# generates kernel.ll
./YourDSL

clang++ -O3 ../main.cpp kernel.ll -o run

./YourDSL 15 6 10 > YourDSL.out
./YourDSL 2 2 1 >> YourDSL.out
./YourDSL 30 12 35 >> YourDSL.out

# generates kernelsol.ll
./YourDSLSol

clang++ -O3 ../solution/main.cpp kernelsol.ll -o runSol

./YourDSLSol 15 6 10 > YourDSLSol.out
./YourDSLSol 2 2 1 >> YourDSLSol.out
./YourDSLSol 30 12 35 >> YourDSLSol.out

if [[ $(cmp YourDSL.out YourDSLSol.out) ]]; then
  echo "YourDSL.out and YourDSLSol.out differ"
  exit 1
fi
echo "YourDSL.out and YourDSLSol.out are the same"
