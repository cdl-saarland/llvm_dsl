#pragma once

#include <functional>
#include <memory>
#include <string>

namespace MyDSL::jit {
namespace detail {

void *open(const std::string &filename);
void close(void *handle);
void *getFunctionPtr(void *handle, const std::string &functionName);

} // namespace detail

using Handle = std::unique_ptr<void, decltype(&detail::close)>;
Handle compile(const std::string &moduleFilename);

template <typename FuncType>
FuncType getFunctionPtr(Handle &handle,
                                       const std::string &functionName) {
  return (FuncType)detail::getFunctionPtr(handle.get(), functionName);
}
} // namespace MyDSL::jit
