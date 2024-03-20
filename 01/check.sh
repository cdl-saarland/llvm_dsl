#!/bin/bash

SCRIPT_PATH=$(dirname $(realpath -s $0))

mkdir -p $SCRIPT_PATH/build && cd $SCRIPT_PATH/build
cmake .. -G Ninja
ninja

./YourDSL 15 > YourDSL.out
./YourDSL 123 >> YourDSL.out
./YourDSL 5678 >> YourDSL.out

./YourDSLSol 15 > YourDSLSol.out
./YourDSLSol 123 >> YourDSLSol.out
./YourDSLSol 5678 >> YourDSLSol.out

if [[ $(cmp YourDSL.out YourDSLSol.out) ]]; then
  echo "YourDSL.out and YourDSLSol.out differ"
  exit 1
fi
echo "YourDSL.out and YourDSLSol.out are the same"
