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

template <class T, class F>
inline void tensor_conv(T *dest_tensor, F &&input_value, T *filter,
                        std::int64_t size, std::int64_t window)
  requires std::invocable<F, int>
{
  // padding for filter
  const std::int64_t offset = window / 2;
  for (std::int64_t i = 0; i < size - offset * 2; i++) {
    for (std::int64_t j = 0; j < size - offset * 2; j++) {
      float acc{0.f};
      for (std::int64_t u = 0; u < window; ++u) {
        for (std::int64_t v = 0; v < window; ++v) {
          auto idx = (i + u) * size + j + v;
          acc += std::forward<F>(input_value)(idx) * filter[u * window + v];
        }
      }
      dest_tensor[i * (size - offset * 2) + j] = acc;
    }
  }
}

extern "C" void __mydsl_tensor_conv_2_f32(float *dest_tensor, float *tensor_a,
                                          float *filter, std::int64_t size,
                                          std::int64_t window) {
  tensor_conv(
      dest_tensor, [tensor_a](std::int64_t idx) { return tensor_a[idx]; },
      filter, size, window);
}

extern "C" void __mydsl_fused_tensor_elementwise_mul_conv_2_f32(
    float *dest_tensor, float *tensor_a, float *tensor_b, float *filter,
    std::int64_t size, std::int64_t window) {
  tensor_conv(
      dest_tensor,
      [tensor_a, tensor_b](std::int64_t idx) {
        return tensor_a[idx] * tensor_b[idx];
      },
      filter, size, window);
}
