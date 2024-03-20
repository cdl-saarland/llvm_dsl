#pragma once

#include <llvm/IR/Constants.h>
#include <llvm/IR/IRBuilder.h>

#include "base_ops.hpp"
#include "bool_ops.hpp"

namespace MyDSL {

class Integer;
class Float;

/**
 * @brief Helper class to introduce common control flow.
 */
class ControlFlow {
  llvm::IRBuilder<> &builder_;

  BaseOps IfImpl(const Bool &Cond, const std::function<BaseOps()> &Then,
                 const std::function<BaseOps()> &Else = nullptr);

public:
  ControlFlow(llvm::IRBuilder<> &builder) : builder_(builder) {}

  /**
   * @brief Emits an if-else statement.
   *
   * @param Cond A Boolean value that determines which branch to take.
   * @param Then A functor that generates the "then" branch.
   * @param Else A (optional) functor that generates the "else" branch.
   */
  void If(const Bool &Cond, const std::function<void()> &Then,
          const std::function<void()> &Else = nullptr);

  /**
   * @brief Emits a while loop.
   *
   * The loop will continue as long as \a Cond is true.
   * The loop body is defined by Body.
   *
   * @param Cond A functor that generates the loop exit condition.
   * @param Body A functor that generates the loop body.
   */
  void While(const std::function<Bool()> &Cond,
             const std::function<void()> &Body);

  /**
   * @brief Emits a for loop with an index starting at \a Start, incrementing by
   * \a Step.
   *
   * The loop will continue as long as \a Cond is true.
   * The loop body is defined by \a Body.
   * Both \a Cond and \a Body receive the current index as an argument.
   *
   * @param Start Integer value as start.
   * @param Cond Functor that generates the loop exit condition. It receives the
   * current loop index as value.
   * @param Step Functor that generates the next loop index. It receives the
   * current loop index as value and returns the new one.
   * @param Body Functor that generates the loop body. It receives the current
   * loop index as value.
   */
  void For(const Integer &Start,
           const std::function<Bool(const Integer &)> &Cond,
           const std::function<Integer(const Integer &)> &Step,
           const std::function<void(const Integer &)> &Body);

  // Emits a return statement.
  void Return();
  // Emits a return statement with a value.
  void Return(const Integer &value);
  void Return(const Float &value);
  void Return(const Bool &value);
};

} // namespace MyDSL
