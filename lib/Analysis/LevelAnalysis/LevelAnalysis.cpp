#include "lib/Analysis/LevelAnalysis/LevelAnalysis.h"

#include "lib/Analysis/SecretnessAnalysis/SecretnessAnalysis.h"
#include "lib/Dialect/Mgmt/IR/MgmtOps.h"
#include "lib/Dialect/Secret/IR/SecretOps.h"
#include "llvm/include/llvm/ADT/TypeSwitch.h"              // from @llvm-project
#include "llvm/include/llvm/Support/Debug.h"               // from @llvm-project
#include "mlir/include/mlir/Analysis/DataFlowFramework.h"  // from @llvm-project
#include "mlir/include/mlir/Dialect/Arith/IR/Arith.h"      // from @llvm-project
#include "mlir/include/mlir/Dialect/Tensor/IR/Tensor.h"    // from @llvm-project
#include "mlir/include/mlir/IR/Operation.h"                // from @llvm-project
#include "mlir/include/mlir/IR/Value.h"                    // from @llvm-project
#include "mlir/include/mlir/IR/Visitors.h"                 // from @llvm-project
#include "mlir/include/mlir/Support/LLVM.h"                // from @llvm-project

#define DEBUG_TYPE "MulResultAnalysis"

namespace mlir {
namespace heir {

LogicalResult LevelAnalysis::visitOperation(
    Operation *op, ArrayRef<const LevelLattice *> operands,
    ArrayRef<LevelLattice *> results) {
  auto propagate = [&](Value value, const LevelState &state) {
    auto *lattice = getLatticeElement(value);
    ChangeResult changed = lattice->join(state);
    propagateIfChanged(lattice, changed);
  };

  auto ensureSecretness = [&](Operation *op, Value value) -> bool {
    // create dependency on SecretnessAnalysis
    auto *lattice =
        getOrCreateFor<SecretnessLattice>(getProgramPointAfter(op), value);
    if (!lattice->getValue().isInitialized()) {
      return false;
    }
    return lattice->getValue().getSecretness();
  };

  llvm::TypeSwitch<Operation &>(*op)
      .Case<secret::GenericOp>([&](auto genericOp) {
        Block *body = genericOp.getBody();
        for (auto i = 0; i != body->getNumArguments(); ++i) {
          auto blockArg = body->getArgument(i);
          propagate(blockArg, LevelState(0));
        }
      })
      .Case<mgmt::ModReduceOp>([&](auto modReduceOp) {
        // implicitly ensure that the operand is secret
        const auto *operandLattice = operands[0];
        if (!operandLattice->getValue().isInitialized()) {
          return;
        }
        auto level = operandLattice->getValue().getLevel();
        propagate(modReduceOp.getResult(), LevelState(level + 1));
      })
      .Default([&](auto &op) {
        if (op.getNumResults() == 0) {
          return;
        }

        // condition on result secretness
        auto secretness = ensureSecretness(&op, op.getResult(0));
        if (!secretness) {
          return;
        }

        auto levelResult = 0;
        for (const auto *operand : operands) {
          auto secretness = ensureSecretness(&op, operand->getAnchor());
          if (!secretness) {
            continue;
          }
          // now operand is secret
          if (!operand->getValue().isInitialized()) {
            return;
          }
          levelResult = std::max(levelResult, operand->getValue().getLevel());
        }

        for (auto result : op.getResults()) {
          propagate(result, LevelState(levelResult));
        }
      });
  return success();
}

}  // namespace heir
}  // namespace mlir
