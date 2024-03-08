#pragma once

#include <llvm/IR/Constants.h>
#include <llvm/IR/IRBuilder.h>

#include "base_ops.hpp"
#include "bool_ops.hpp"

namespace MyDSL {

class Integer;
class Float;

class ControlFlow {
  llvm::IRBuilder<> &builder_;

  BaseOps IfImpl(Bool Cond, const std::function<BaseOps()> &Then,
                 const std::function<BaseOps()> &Else = nullptr);

public:
  ControlFlow(llvm::IRBuilder<> &builder) : builder_(builder) {}

  void If(Bool Cond, const std::function<void()> &Then,
          const std::function<void()> &Else = nullptr);

  template <class RetT>
  RetT If(Bool Cond, const std::function<RetT()> &Then,
          const std::function<RetT()> &Else = nullptr) {
    return static_cast<RetT>(IfImpl(
        Cond, [&]() -> BaseOps { return Then(); },
        [&]() -> BaseOps { return Else(); }));
  }

  void While(const std::function<Bool()> &Cond,
             const std::function<void()> &Body);

  void For(const Integer &Start, const Bool &Cond,
           const std::function<void()> &Body,
           const std::function<void()> &Step);
  void Break();
  void Continue();
  void Return();
  void Return(Integer value);
  void Return(Float value);
  void Return(Bool value);
};

} // namespace MyDSL
