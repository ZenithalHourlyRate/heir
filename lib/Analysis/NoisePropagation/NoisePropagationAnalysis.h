#ifndef INCLUDE_ANALYSIS_NOISEPROPAGATION_NOISEPROPAGATIONANALYSIS_H_
#define INCLUDE_ANALYSIS_NOISEPROPAGATION_NOISEPROPAGATIONANALYSIS_H_

#include "lib/Analysis/NoisePropagation/NoiseKPZ21.h"
#include "lib/Analysis/NoisePropagation/Params.h"
#include "lib/Analysis/NoisePropagation/Variance.h"
#include "mlir/include/mlir/Analysis/DataFlow/SparseAnalysis.h"  // from @llvm-project
#include "mlir/include/mlir/IR/Operation.h"  // from @llvm-project
#include "mlir/include/mlir/IR/Value.h"      // from @llvm-project

namespace mlir {
namespace heir {

#define NoiseType Variance

/// This lattice element represents the noise distribution of an SSA value.
class NoiseLattice : public dataflow::Lattice<NoiseType> {
 public:
  using Lattice::Lattice;
};

class NoiseAnalysis
    : public dataflow::SparseForwardDataFlowAnalysis<NoiseLattice> {
 public:
  using SparseForwardDataFlowAnalysis::SparseForwardDataFlowAnalysis;

  void setToEntryState(NoiseLattice *lattice) override {
    // At an entry point, we have no information about the noise.
    propagateIfChanged(lattice, lattice->join(NoiseType::uninitialized()));
  }

  LogicalResult visitOperation(Operation *op,
                               ArrayRef<const NoiseLattice *> operands,
                               ArrayRef<NoiseLattice *> results) override;
};

}  // namespace heir
}  // namespace mlir

#endif  // INCLUDE_ANALYSIS_NOISEPROPAGATION_NOISEPROPAGATIONANALYSIS_H_
