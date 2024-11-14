#include "lib/Analysis/NoisePropagation/NoisePropagationAnalysis.h"

#include "lib/Dialect/BGV/IR/BGVOps.h"
#include "llvm/include/llvm/ADT/TypeSwitch.h"  // from @llvm-project
#include "llvm/include/llvm/Support/Debug.h"   // from @llvm-project
#include "mlir/include/mlir/IR/Operation.h"    // from @llvm-project
#include "mlir/include/mlir/IR/Value.h"        // from @llvm-project

#define DEBUG_TYPE "NoisePropagationAnalysis"

namespace mlir {
namespace heir {

LogicalResult NoiseStatesAnalysis::visitOperation(
    Operation *op, ArrayRef<const VarianceStatesLattice *> operands,
    ArrayRef<VarianceStatesLattice *> results) {
  // auto noisePropagationOp = dyn_cast<NoisePropagationInterface>(op);
  // if (!noisePropagationOp) {
  //   setAllToEntryStates(results);
  //   return success();
  // }

  // LLVM_DEBUG(llvm::dbgs() << "Visiting " << *op << "\n");

  VarianceStates vss;

  llvm::TypeSwitch<Operation &>(*op)
      .Case<bgv::MyEncryptOp>([&](auto encryptOp) {
        vss = VarianceStates::evalEncryptPk(op->getResult(0), 65537, 3);
        // LLVM_DEBUG(llvm::dbgs() << "Encrypted states " << vss << "\n");
      })
      .Case<bgv::MyMulOp>([&](auto mulOp) {
        vss = VarianceStates::evalMultNoRelin(
            operands[0]->getValue(), operands[1]->getValue(), op->getResult(0));
        // LLVM_DEBUG(llvm::dbgs() << " Mul states " << vss << "\n");
      });

  // LLVM_DEBUG(llvm::dbgs() << "join start\n");
  VarianceStatesLattice *lattice = results[0];
  VarianceStates old = lattice->getValue();
  ChangeResult changed = lattice->join(vss);
  // LLVM_DEBUG(llvm::dbgs() << "join end\n");
  propagateIfChanged(lattice, changed);
  return success();
}

}  // namespace heir
}  // namespace mlir
