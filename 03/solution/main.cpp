#include "int_ops.hpp"

#include <algorithm>
#include <iostream>
#include <memory>
#include <numeric>
#include <string>

using namespace MyDSL;
Integer checksum(Integer i) {
  Integer sum = 0;
  while (i > Integer(0)) {
    sum += i % 10;
    i /= 10;
  }
  return sum;
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
