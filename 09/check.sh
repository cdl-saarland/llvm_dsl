#!/bin/bash

SCRIPT_PATH=$(dirname $(realpath -s $0))

mkdir -p $SCRIPT_PATH/build && cd $SCRIPT_PATH/build
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
ninja

./YourDSL > YourDSL.out 2> YourDSL.err

./YourDSLSol > YourDSLSol.out

if [[ $(cmp YourDSL.out YourDSLSol.out) ]]; then
  echo "YourDSL.out and YourDSLSol.out differ"
  exit 1
fi
echo "YourDSL.out and YourDSLSol.out are the same"

grep "__mydsl_fused_tensor_elementwise_mul_conv_2_f32" YourDSL.err > /dev/null
if [[ $? -ne 0 ]]; then
    echo "There's no __mydsl_fused_tensor_elementwise_mul_conv_2_f32 used!"
fi
