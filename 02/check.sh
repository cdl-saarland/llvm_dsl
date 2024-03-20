#!/bin/bash

SCRIPT_PATH=$(dirname $(realpath -s $0))

mkdir -p $SCRIPT_PATH/build && cd $SCRIPT_PATH/build
cmake .. -G Ninja
ninja

./YourDSL 15 35 10 > YourDSL.out
./YourDSL 2 2 1 >> YourDSL.out
./YourDSL 100 12 35 >> YourDSL.out

./YourDSLSol 15 35 10 > YourDSLSol.out
./YourDSLSol 2 2 1 >> YourDSLSol.out
./YourDSLSol 100 12 35 >> YourDSLSol.out

if [[ $(cmp YourDSL.out YourDSLSol.out) ]]; then
  echo "YourDSL.out and YourDSLSol.out differ"
  exit 1
fi
echo "YourDSL.out and YourDSLSol.out are the same"
