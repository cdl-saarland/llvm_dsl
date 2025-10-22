#!/bin/bash

cd 01
./build_all.sh

cd ../03
rm -rf build
./check.sh
cd ../04
rm -rf build
./check.sh
cd ../05
rm -rf build
./check.sh
cd ../06
rm -rf build
./check.sh
cd ../07
rm -rf build
./check.sh
cd ../08
rm -rf build
./check.sh
cd ../09
rm -rf build
./check.sh
