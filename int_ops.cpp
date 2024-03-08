#include "int_ops.hpp"

#include "float_ops.hpp"

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

namespace MyDSL {
Integer::operator Float() const {
  return {builder_.CreateSIToFP(getValue(),
                                llvm::Type::getFloatTy(builder_.getContext())),
          builder_};
}

} // namespace MyDSL
