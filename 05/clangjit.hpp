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

// TODO! Implement me!
// Should compile the given moduleFilename (LLVM IR file) to a shared library
// and return a Handle to the loaded shared library.
Handle compile(const std::string &moduleFilename);

// Get a function pointer of the given functionName from the shared library
// represented by handle.
template <typename FuncType>
FuncType getFunctionPtr(Handle &handle, const std::string &functionName) {
  return (FuncType)detail::getFunctionPtr(handle.get(), functionName);
}
} // namespace MyDSL::jit
