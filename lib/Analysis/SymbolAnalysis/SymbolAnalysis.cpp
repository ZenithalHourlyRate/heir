#include "lib/Analysis/SymbolAnalysis/SymbolAnalysis.h"

#include <algorithm>
#include <cassert>
#include <functional>

#include "lib/Analysis/SecretnessAnalysis/SecretnessAnalysis.h"
#include "lib/Analysis/Utils.h"
#include "lib/Dialect/Secret/IR/SecretOps.h"
#include "llvm/include/llvm/ADT/TypeSwitch.h"              // from @llvm-project
#include "llvm/include/llvm/Support/Debug.h"               // from @llvm-project
#include "mlir/include/mlir/Analysis/DataFlowFramework.h"  // from @llvm-project
#include "mlir/include/mlir/IR/Operation.h"                // from @llvm-project
#include "mlir/include/mlir/IR/Value.h"                    // from @llvm-project
#include "mlir/include/mlir/IR/Visitors.h"                 // from @llvm-project
#include "mlir/include/mlir/Interfaces/CallInterfaces.h"   // from @llvm-project
#include "mlir/include/mlir/Support/LLVM.h"                // from @llvm-project

#define DEBUG_TYPE "SymbolAnalysis"

namespace mlir {
namespace heir {

LogicalResult SymbolAnalysis::visitOperation(
    Operation *op, ArrayRef<const SymbolLattice *> operands,
    ArrayRef<SymbolLattice *> results) {
  auto propagate = [&](Value value, const SymbolState &state) {
    auto *lattice = getLatticeElement(value);
    ChangeResult changed = lattice->join(state);
    LLVM_DEBUG(llvm::dbgs()
               << "Propagating " << state << " to " << value << "\n");
    propagateIfChanged(lattice, changed);
  };

  llvm::TypeSwitch<Operation &>(*op)
      .Case<secret::GenericOp>([&](auto genericOp) {
        Block *body = genericOp.getBody();
        for (auto i = 0; i != body->getNumArguments(); ++i) {
          auto blockArg = body->getArgument(i);
          propagate(blockArg, SymbolState("k" + std::to_string(counter++)));
        }
      })
      .Default([&](auto &op) {
        // condition on result secretness
        SmallVector<OpResult> secretResults;
        getSecretResults(&op, secretResults);
        if (secretResults.empty()) {
          return;
        }

        for (auto result : secretResults) {
          propagate(result, SymbolState("k" + std::to_string(counter++)));
        }
      });

  return success();
}

void SymbolAnalysis::visitExternalCall(
    CallOpInterface call, ArrayRef<const SymbolLattice *> argumentLattices,
    ArrayRef<SymbolLattice *> resultLattices) {
  auto callback = std::bind(&SymbolAnalysis::propagateIfChangedWrapper, this,
                            std::placeholders::_1, std::placeholders::_2);
  ::mlir::heir::visitExternalCall<SymbolState, SymbolLattice>(
      call, argumentLattices, resultLattices, callback);
}

}  // namespace heir
}  // namespace mlir
