#ifndef INCLUDE_ANALYSIS_NOISEPROPAGATION_PARAMANALYSIS_H_
#define INCLUDE_ANALYSIS_NOISEPROPAGATION_PARAMANALYSIS_H_

#include "lib/Analysis/NoisePropagation/Params.h"
#include "mlir/include/mlir/Analysis/DataFlow/SparseAnalysis.h"  // from @llvm-project
#include "mlir/include/mlir/IR/Operation.h"  // from @llvm-project
#include "mlir/include/mlir/IR/Value.h"      // from @llvm-project

namespace mlir {
namespace heir {

class LocalParamState {
 public:
  LocalParamState() : param(std::nullopt) {}
  explicit LocalParamState(LocalParam param) : param(param) {}
  ~LocalParamState() = default;

  LocalParam getLocalParam() const {
    assert(isInitialized());
    return param.value();
  }

  bool operator==(const LocalParamState &rhs) const {
    return param == rhs.param;
  }

  bool isInitialized() const { return param.has_value(); }

  static LocalParamState join(const LocalParamState &lhs,
                              const LocalParamState &rhs) {
    if (!lhs.isInitialized()) return rhs;
    if (!rhs.isInitialized()) return lhs;

    if (lhs.getLocalParam() == rhs.getLocalParam()) return lhs;

    return LocalParamState{lhs.getLocalParam()};
  }

  void print(llvm::raw_ostream &os) const {
    if (isInitialized()) {
      os << "LocalParamState(" << param.value() << ")";
    } else {
      os << "LocalParamState(uninitialized)";
    }
  }

 private:
  std::optional<LocalParam> param;
};

/// This lattice element represents the noise distribution of an SSA value.
class ParamLattice : public dataflow::Lattice<LocalParamState> {
 public:
  using Lattice::Lattice;
};

class ParamAnalysis
    : public dataflow::SparseForwardDataFlowAnalysis<ParamLattice> {
 public:
  using SparseForwardDataFlowAnalysis::SparseForwardDataFlowAnalysis;

  void setToEntryState(ParamLattice *lattice) override {
    // At an entry point, we have no information about the noise.
    propagateIfChanged(lattice, lattice->join(LocalParamState()));
  }

  LogicalResult visitOperation(Operation *op,
                               ArrayRef<const ParamLattice *> operands,
                               ArrayRef<ParamLattice *> results) override;
};

}  // namespace heir
}  // namespace mlir

#endif  // INCLUDE_ANALYSIS_NOISEPROPAGATION_PARAMANALYSIS_H_
