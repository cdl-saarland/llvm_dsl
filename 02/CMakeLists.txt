cmake_minimum_required(VERSION 3.5)

project(YourDSL)
find_package(LLVM REQUIRED)
include(AddLLVM)

add_llvm_executable(YourDSL
  main.cpp
  jit.cpp
  int_ops.cpp
  float_ops.cpp
)
set_property(TARGET YourDSL PROPERTY CXX_STANDARD 20)

target_include_directories(YourDSL PUBLIC ${LLVM_INCLUDE_DIR})
target_link_libraries(YourDSL PUBLIC LLVM)

add_llvm_executable(YourDSLSol
  solution/main.cpp
  solution/jit.cpp
  solution/int_ops.cpp
  solution/float_ops.cpp

  PARTIAL_SOURCES_INTENDED
)
set_property(TARGET YourDSLSol PROPERTY CXX_STANDARD 20)

target_include_directories(YourDSLSol PUBLIC ${LLVM_INCLUDE_DIR})
target_link_libraries(YourDSLSol PUBLIC LLVM)
