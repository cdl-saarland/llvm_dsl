#include <algorithm>
#include <iostream>
#include <memory>
#include <numeric>
#include <string>

extern "C" float kernel(float a, float b, float c);

int main(int argc, const char *argv[]) {
  if (argc < 4) {
    std::cerr << "Usage: " << argv[0] << " <a> <b> <c>\n";
    return 1;
  }

  float A = std::stof(argv[1]);
  float B = std::stof(argv[2]);
  float C = std::stof(argv[3]);

  auto Ret = kernel(A, B, C);

  std::cout << "Evaluated to: " << Ret << "\n";

  return 0;
}
