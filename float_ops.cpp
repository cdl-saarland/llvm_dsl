#include "int_ops.hpp"

#include "float_ops.hpp"

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

namespace MyDSL {
Float::operator Integer() const {
  return {builder_.CreateFPToSI(value_,
                                llvm::Type::getInt64Ty(builder_.getContext())),
          builder_};
}

} // namespace MyDSL
