// clang++ -O3 01.cpp -o 01 && ./01 1 2

#include <iostream>

int main(int argc, const char *argv[]) {
  std::cout << "Welcome to the LLVM Demystified Workshop!\nGot " << argc - 1 << " arguments\n";
  return 0;
}
