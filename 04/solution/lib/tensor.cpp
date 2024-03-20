#include <cstdint>

extern "C" void __mydsl_tensor_mmul_2_f32(float *dest_tensor, float *tensor_a,
                                          float *tensor_b, std::int64_t size) {

  for (std::int64_t i = 0; i < size; ++i)
    for (std::int64_t j = 0; j < size; ++j) {
      float acc = 0.f;
      for (std::int64_t k = 0; k < size; ++k)
        acc += tensor_a[i * size + k] * tensor_b[k * size + j];
      dest_tensor[i * size + j] = acc;
    }
}
