#ifndef INCLUDE_ANALYSIS_NOISEPROPAGATION_NOISEPROPAGATIONANALYSIS_H_
#define INCLUDE_ANALYSIS_NOISEPROPAGATION_NOISEPROPAGATIONANALYSIS_H_

#include "lib/Analysis/NoisePropagation/Params.h"
#include "lib/Analysis/NoisePropagation/Variance.h"
#include "mlir/include/mlir/Analysis/DataFlow/SparseAnalysis.h"  // from @llvm-project
#include "mlir/include/mlir/IR/Operation.h"  // from @llvm-project
#include "mlir/include/mlir/IR/Value.h"      // from @llvm-project

namespace mlir {
namespace heir {

/// This lattice element represents the noise distribution of an SSA value.
class VarianceStatesLattice : public dataflow::Lattice<VarianceStates> {
 public:
  using Lattice::Lattice;
};

class NoiseStatesAnalysis
    : public dataflow::SparseForwardDataFlowAnalysis<VarianceStatesLattice> {
 public:
  using SparseForwardDataFlowAnalysis::SparseForwardDataFlowAnalysis;

  void setToEntryState(VarianceStatesLattice *lattice) override {
    // At an entry point, we have no information about the noise.
    propagateIfChanged(lattice, lattice->join(VarianceStates()));
  }

  LogicalResult visitOperation(
      Operation *op, ArrayRef<const VarianceStatesLattice *> operands,
      ArrayRef<VarianceStatesLattice *> results) override;
};

}  // namespace heir
}  // namespace mlir

#endif  // INCLUDE_ANALYSIS_NOISEPROPAGATION_NOISEPROPAGATIONANALYSIS_H_
