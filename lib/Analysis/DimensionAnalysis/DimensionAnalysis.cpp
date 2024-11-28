#include "lib/Analysis/DimensionAnalysis/DimensionAnalysis.h"

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

LogicalResult DimensionAnalysis::visitOperation(
    Operation *op, ArrayRef<const DimensionLattice *> operands,
    ArrayRef<DimensionLattice *> results) {
  auto propagate = [&](Value value, const DimensionState &state) {
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
          propagate(blockArg, DimensionState(2));
        }
      })
      // NOTE: special case for ExtractOp...
      .Case<mgmt::RelinearizeOp, tensor::ExtractOp>([&](auto relinearizeOp) {
        // implicitly ensure that the operand is secret
        propagate(relinearizeOp.getResult(), DimensionState(2));
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

        auto isMul = false;
        if (isa<arith::MulIOp, arith::MulFOp>(op)) {
          isMul = true;
        }

        auto dimensionResult = 0;
        for (const auto *operand : operands) {
          auto secretness = ensureSecretness(&op, operand->getAnchor());
          // pt/ct?
          auto dimension = 2;
          if (secretness) {
            if (!operand->getValue().isInitialized()) {
              return;
            }
            // ct
            dimension = operand->getValue().getDimension();
          }

          if (isMul) {
            dimensionResult += dimension;
          } else {
            dimensionResult = std::max(dimensionResult, dimension);
          }
        }
        // tensor product
        if (isMul) {
          dimensionResult -= 1;
        }

        for (auto result : op.getResults()) {
          propagate(result, DimensionState(dimensionResult));
        }
      });
  return success();
}

}  // namespace heir
}  // namespace mlir
