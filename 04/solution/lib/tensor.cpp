#include <concepts>
#include <cstdint>
#include <utility>

extern "C" void __mydsl_tensor_elementwise_mul_2_f32(float *dest_tensor,
                                                     float *tensor_a,
                                                     float *tensor_b,
                                                     std::int64_t size) {
  for (int i = 0; i < size * size; i++) {
    dest_tensor[i] = tensor_a[i] * tensor_b[i];
  }
}

extern "C" void __mydsl_tensor_elementwise_div_2_f32(float *dest_tensor,
                                                     float *tensor_a,
                                                     float *tensor_b,
                                                     std::int64_t size) {
  for (int i = 0; i < size * size; i++) {
    dest_tensor[i] = tensor_a[i] / tensor_b[i];
  }
}

extern "C" void __mydsl_tensor_elementwise_add_2_f32(float *dest_tensor,
                                                     float *tensor_a,
                                                     float *tensor_b,
                                                     std::int64_t size) {
  for (int i = 0; i < size * size; i++) {
    dest_tensor[i] = tensor_a[i] + tensor_b[i];
  }
}

extern "C" void __mydsl_tensor_elementwise_sub_2_f32(float *dest_tensor,
                                                     float *tensor_a,
                                                     float *tensor_b,
                                                     std::int64_t size) {
  for (int i = 0; i < size * size; i++) {
    dest_tensor[i] = tensor_a[i] + tensor_b[i];
  }
}

