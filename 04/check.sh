#!/bin/bash

SCRIPT_PATH=$(dirname $(realpath -s $0))

mkdir -p $SCRIPT_PATH/build && cd $SCRIPT_PATH/build
cmake .. -G Ninja
ninja

./YourDSL 2.4 5.1 10 > YourDSL.out
./YourDSL 2.7 1.1 15 >> YourDSL.out
./YourDSL 28.4 7.5 100 >> YourDSL.out

./YourDSLSol 2.4 5.1 10 > YourDSLSol.out
./YourDSLSol 2.7 1.1 15 >> YourDSLSol.out
./YourDSLSol 28.4 7.5 100 >> YourDSLSol.out

if [[ $(cmp YourDSL.out YourDSLSol.out) ]]; then
  echo "YourDSL.out and YourDSLSol.out differ"
  exit 1
fi
echo "YourDSL.out and YourDSLSol.out are the same"
