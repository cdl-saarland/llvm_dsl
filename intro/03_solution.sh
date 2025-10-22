#! /bin/bash

clang++ -O -Xclang -disable-llvm-optzns -fno-discard-value-names -g0 03.cpp -S -emit-llvm -o 03.ll
opt -passes=sroa 03.ll -o 03_sroa.ll -S
sed -i "s/%add = add i64 %acc.0, %mul/%add = mul i64 %acc.0, %mul/g" 03_sroa.ll
sed -i "s/%acc.0 = phi i64 \[ 0, %entry \], \[ %add, %for.inc \]/%acc.0 = phi i64 \[ 1, %entry \], \[ %add, %for.inc \]/g" 03_sroa.ll

lli 03_sroa.ll 4
clang++ 03_sroa.ll -o 03_mul
./03_mul 2
