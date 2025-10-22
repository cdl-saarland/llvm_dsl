cmake_minimum_required(VERSION 3.5)

project(YourDSL)

add_executable(YourDSL
  main.cpp
)
set_property(TARGET YourDSL PROPERTY CXX_STANDARD 20)

add_executable(YourDSLSol
  solution/main.cpp
)
set_property(TARGET YourDSLSol PROPERTY CXX_STANDARD 20)
