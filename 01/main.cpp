#include "int_ops.hpp"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <numeric>
#include <string>

using namespace MyDSL;
Integer checksum(Integer i) {
  // todo: implement checksum algorithm..
  return i;
}

int main(int argc, const char *argv[]) {
  if (argc < 2) {
    std::cout << "Usage: " << argv[0] << " <number>" << std::endl;
    return 1;
  }

  auto number = std::stoll(argv[1]);

  std::cout << "The checksum of " << number << " is " << checksum(number)
            << std::endl;

  return 0;
}
