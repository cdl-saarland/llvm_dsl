#! /bin/bash

clang++ -O -Xclang -disable-llvm-optzns -g0 02.cpp -emit-llvm -fno-discard-value-names -c -o 02.bc # Parse C++ to LLVM IR
llvm-dis 02.bc -o 02.ll                                                   # Disassemble IR (skip: -c -> -S above)
llvm-extract --func=_Z4relul 02.ll -o 02_relu.ll -S                       # Extract relu
llvm-extract --func=main 02.ll -o 02_main.ll -S --keep-const-init         # Extract main (and const inits)
opt 02_relu.ll -passes="sroa,simplifycfg" -S -o 02_relu_opt.ll            # Simplify relu
llvm-link 02_relu_opt.ll 02_main.ll -o 02_full.ll -S                      # Link optimized relu and main again
llvm-as 02_full.ll -o 02_full.bc                                          # Assemble IR to bitcode (optional)

# Run using interpreter
lli 02_full.bc 10

# Compile BC to assembly
llc 02_full.bc -o 02_full.s

# Compile BC (and link) to executable
clang 02_full.bc -o 02_full
./02_full
