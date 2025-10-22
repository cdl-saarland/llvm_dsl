#include "clangjit.hpp"

#include "dlfcn.h"

#include <cstdlib>
#include <iostream>
#include <sstream>

namespace MyDSL::jit {
namespace detail {

void *open(const std::string &file) {
  if (void *handle = dlopen(file.c_str(), RTLD_NOW))
    return handle;
  else if (auto err = dlerror())
    std::cerr << "could not load shared lib '" << file << "' due to error '"
              << err << "'" << std::endl;
  else
    std::cerr << "could not load shared lib for unknown reason: " << file
              << std::endl;
  return nullptr;
}

void close(void *handle) {
  if (auto err = dlclose(handle))
    std::cerr << "could not close handle" << std::endl;
}

void *getFunctionPtr(void *handle, const std::string &symbol) {
  dlerror(); // clear error state
  void *addr = dlsym(handle, symbol.c_str());
  if (auto err = dlerror()) {
    std::cerr << "could not find symbol '" << symbol
              << "' in plugin due to error '" << err << "'" << std::endl;
    return nullptr;
  } else
    return addr;
}
} // namespace detail

Handle compile(const std::string &moduleFilename) {
  std::string kernel_so = "/tmp/" + moduleFilename + ".so";
  std::stringstream cmd;
  
  // todo: create cmd to compile moduleFilename to kernel_so
  
  if (auto error = WEXITSTATUS(std::system(cmd.str().c_str())) != 0) {
    return {nullptr, &detail::close};
  }

  // todo: return handle to compiled shared library
  return {nullptr, &detail::close};
}
} // namespace MyDSL::jit
