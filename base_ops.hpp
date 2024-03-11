#pragma once

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instructions.h>

namespace MyDSL {

/**
 * @brief Base class for types that represent a value in the DSL.
 * The base class provides the basic infrastructure for the values, such as
 * memory allocation, load/store operations, assignment and streaming operators.
 */
class BaseOps {
protected:
  llvm::IRBuilder<> &builder_;
  llvm::AllocaInst *mem_;

  void store(llvm::Value *value) { builder_.CreateStore(value, mem_); }

  llvm::Value *load() const {
    return builder_.CreateLoad(mem_->getAllocatedType(), mem_);
  }

public:
  /**
   * @brief Construct a new Base Ops object. Stores the value to a new Alloca.
   *
   * @param value The initial value for the Alloca.
   * @param builder The builder to use for emitting operations.
   */
  BaseOps(llvm::Value *value, llvm::IRBuilder<> &builder) : builder_(builder) {
    auto &EntryBB = builder.GetInsertBlock()->getParent()->getEntryBlock();

    mem_ =
        llvm::IRBuilder<>{&EntryBB, EntryBB.getFirstInsertionPt()}.CreateAlloca(
            value->getType());
    if (!llvm::isa<llvm::UndefValue>(value))
      store(value);
  }

  /**
   * @brief Construct a new Base Ops object. Creates a new Alloca of the given
   * type without initializing it.
   *
   * @param Ty The type of the Alloca.
   * @param builder The builder to use for emitting operations.
   */
  BaseOps(llvm::Type *Ty, llvm::IRBuilder<> &builder)
      : BaseOps(llvm::UndefValue::get(Ty), builder) {}

  /**
   * @brief Copy operator.
   * Creates a new Alloca and stores the value of the other object.
   *
   * @param other The other object to copy the value from.
   */
  BaseOps(const BaseOps &other) : BaseOps(other.getValue(), other.builder_) {}

  /**
   * @brief Move constructor.
   * The other object is invalidated.
   *
   * @param other The other object to move the value from.
   */
  BaseOps(BaseOps &&other) : builder_(other.builder_), mem_(other.mem_) {
    other.mem_ = nullptr;
  }

  /**
   * @brief Copy operator.
   * Stores the value of the other object to the current alloca.
   *
   * @param other The other object to copy the value from.
   * @return BaseOps& this.
   */
  BaseOps &operator=(const BaseOps &other) {
    assert(getType() == other.getType() && "Type mismatch");
    store(other.getValue());
    return *this;
  }

  /**
   * @brief Get the Type of the managed value.
   *
   * @return llvm::Type* The type.
   */
  llvm::Type *getType() const { return mem_->getAllocatedType(); }

  /**
   * @brief Get the managed value.
   * Inserts a load from the alloca.
   *
   * @return llvm::Value* The value.
   */
  llvm::Value *getValue() const { return load(); }
  /// Enable implicit use of DSL values as llvm::Value*. Returns the value
  /// returned by #getValue().
  operator llvm::Value *() const { return getValue(); }

  /**
   * @brief Writes the value to the given stream.
   *
   * @param os The stream
   * @param b The value to write
   * @return std::ostream& The stream
   */
  friend std::ostream &operator<<(std::ostream &os, const BaseOps &b) {
    std::string str;
    llvm::raw_string_ostream ros(str);
    ros << *b.getValue();
    os << str;
    return os;
  }
};

} // namespace MyDSL
