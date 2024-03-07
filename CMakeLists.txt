cmake_minimum_required(VERSION 3.5)

project(YourDSL)
find_package(LLVM REQUIRED)
include(AddLLVM)

add_llvm_executable(YourDSL
  main.cpp
  control_flow.cpp
  int_ops.cpp
  float_ops.cpp
)

target_include_directories(YourDSL PUBLIC ${LLVM_INCLUDE_DIR})
target_link_libraries(YourDSL PUBLIC LLVM)

install(TARGETS ${name} RUNTIME DESTINATION "${LLVM_EXAMPLES_INSTALL_DIR}")
