// clang-format off
// clang++ -O3 03.cpp -o 03 && ./03 100
// clang++ -O -Xclang -disable-llvm-optzns -fno-discard-value-names -g0 03.cpp -S -emit-llvm -o 03.ll
// clang-format on

#include <cstdint>
#include <iostream>
#include <numeric>
#include <vector>

std::uint64_t inner_product(const std::uint64_t *a, const std::uint64_t *b,
                            const std::size_t size) {
  std::uint64_t acc = 0;
  for (int i = 0; i < size; ++i)
    acc += a[i] * b[i];

  return acc;
}

int main(int argc, const char *argv[]) {
  std::size_t size = 100;
  if (argc > 1)
    size = std::atoll(argv[1]);
  std::vector<std::uint64_t> a(size);
  std::vector<std::uint64_t> b(size);

  // init a: 1-100, b: 101-200
  std::iota(a.begin(), a.end(), 1);
  std::iota(b.begin(), b.end(), size + 1);

  const std::uint64_t acc = inner_product(a.data(), b.data(), size);

  std::cout << "a o b = " << acc << "\n";
  return 0;
}
