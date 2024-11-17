#include "lib/Analysis/NoisePropagation/NoisePropagationAnalysis.h"

#include "lib/Dialect/Secret/IR/SecretOps.h"
#include "llvm/include/llvm/ADT/TypeSwitch.h"          // from @llvm-project
#include "llvm/include/llvm/Support/Debug.h"           // from @llvm-project
#include "mlir/include/mlir/Dialect/Arith/IR/Arith.h"  // from @llvm-project
#include "mlir/include/mlir/IR/Operation.h"            // from @llvm-project
#include "mlir/include/mlir/IR/Value.h"                // from @llvm-project

#define DEBUG_TYPE "NoisePropagationAnalysis"

namespace mlir {
namespace heir {

LogicalResult NoiseStatesAnalysis::visitOperation(
    Operation *op, ArrayRef<const VarianceStatesLattice *> operands,
    ArrayRef<VarianceStatesLattice *> results) {
  auto propagate = [&](Value value, VarianceStates &vss) {
    // LLVM_DEBUG(llvm::dbgs() << "join before size " << vss.size() << " " <<
    // vss.getResult() << "\n");
    auto *lattice = getLatticeElement(value);
    ChangeResult changed = lattice->join(vss);
    propagateIfChanged(lattice, ChangeResult::Change);
    // LLVM_DEBUG(llvm::dbgs() << "join result size " <<
    // lattice->getValue().size() << " " << lattice->getValue().getResult() <<
    // "\n");
  };

  llvm::TypeSwitch<Operation &>(*op)
      .Case<secret::GenericOp>([&](auto genericOp) {
        Block *body = genericOp.getBody();
        LLVM_DEBUG(llvm::dbgs() << "Visiting secret genericOp with block arg "
                                << body->getArguments().size() << "\n");
        auto maxMulDepth = 0;
        if (auto depthAttr =
                llvm::dyn_cast<IntegerAttr>(genericOp->getAttr("depth"))) {
          maxMulDepth = depthAttr.getValue().getLimitedValue();
        }
        for (Value &arg : body->getArguments()) {
          auto vss = VarianceStates::evalEncryptPk(65537, maxMulDepth);
          // LLVM_DEBUG(llvm::dbgs() << "enc value " << arg << " contained " <<
          // vss.getResult() << " vss " << &vss << "\n");
          propagate(arg, vss);
        }
      })
      .Case<arith::MulIOp>([&](auto mulOp) {
        LLVM_DEBUG(llvm::dbgs() << "Visiting mult op " << mulOp << "\n");
        auto vss = VarianceStates::evalMultNoRelin(operands[0]->getValue(),
                                                   operands[1]->getValue());
        propagate(op->getResult(0), vss);
      });
  return success();
}

}  // namespace heir
}  // namespace mlir
