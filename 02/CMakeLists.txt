cmake_minimum_required(VERSION 3.5)

project(YourDSL)
find_package(LLVM REQUIRED)
include(AddLLVM)

add_llvm_executable(YourDSL
  main.cpp
  jit.cpp
  control_flow.cpp
  int_ops.cpp
  float_ops.cpp
  passes/fuse_ops.cpp
  passes/strip_nooptmd.cpp
)
set_property(TARGET YourDSL PROPERTY CXX_STANDARD 20)

target_include_directories(YourDSL PUBLIC ${LLVM_INCLUDE_DIR})
target_link_libraries(YourDSL PUBLIC LLVM)

# target_compile_options(YourDSL PRIVATE -fsanitize=undefined)
# target_link_options(YourDSL PRIVATE -fsanitize=undefined)

add_subdirectory(lib)
