cmake_minimum_required(VERSION 3.4.3)

project(YourDSL)
find_package(LLVM REQUIRED)
include(AddLLVM)

add_llvm_executable(YourDSL
  main.cpp
  jit.cpp
  codegen.cpp
)

target_include_directories(YourDSL PUBLIC ${LLVM_INCLUDE_DIR})
target_link_libraries(YourDSL PUBLIC LLVM)

install(TARGETS ${name} RUNTIME DESTINATION "${LLVM_EXAMPLES_INSTALL_DIR}")
