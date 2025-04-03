#ifndef LIB_ANALYSIS_SYMBOLANALYSIS_SYMBOLANALYSIS_H_
#define LIB_ANALYSIS_SYMBOLANALYSIS_SYMBOLANALYSIS_H_

#include <algorithm>
#include <cassert>
#include <optional>

#include "lib/Analysis/SecretnessAnalysis/SecretnessAnalysis.h"
#include "llvm/include/llvm/Support/raw_ostream.h"  // from @llvm-project
#include "mlir/include/mlir/Analysis/DataFlow/SparseAnalysis.h"  // from @llvm-project
#include "mlir/include/mlir/Analysis/DataFlowFramework.h"  // from @llvm-project
#include "mlir/include/mlir/IR/Operation.h"                // from @llvm-project
#include "mlir/include/mlir/IR/Value.h"                    // from @llvm-project
#include "mlir/include/mlir/Interfaces/CallInterfaces.h"   // from @llvm-project
#include "mlir/include/mlir/Support/LLVM.h"                // from @llvm-project

namespace mlir {
namespace heir {

// This analysis should be used after --mlir-to-secret-arithmetic
// but before --secret-distribute-generic
// where a whole secret::GenericOp is assumed

class SymbolState {
 public:
  using SymbolType = std::string;

  SymbolState() : symbol(std::nullopt) {}
  explicit SymbolState(SymbolType symbol) : symbol(symbol) {}
  ~SymbolState() = default;

  SymbolType getSymbol() const {
    assert(isInitialized());
    return symbol.value();
  }
  SymbolType get() const { return getSymbol(); }

  bool operator==(const SymbolState &rhs) const { return symbol == rhs.symbol; }

  bool isInitialized() const { return symbol.has_value(); }

  static SymbolState join(const SymbolState &lhs, const SymbolState &rhs) {
    if (!lhs.isInitialized()) return rhs;
    if (!rhs.isInitialized()) return lhs;

    // should be equal then
    return lhs;
  }

  void print(llvm::raw_ostream &os) const {
    if (isInitialized()) {
      os << "SymbolState(" << symbol.value() << ")";
    } else {
      os << "SymbolState(uninitialized)";
    }
  }

  friend llvm::raw_ostream &operator<<(llvm::raw_ostream &os,
                                       const SymbolState &state) {
    state.print(os);
    return os;
  }

 private:
  std::optional<SymbolType> symbol;
};

class SymbolLattice : public dataflow::Lattice<SymbolState> {
 public:
  using Lattice::Lattice;
};

class SymbolAnalysis
    : public dataflow::SparseForwardDataFlowAnalysis<SymbolLattice>,
      public SecretnessAnalysisDependent<SymbolAnalysis> {
 public:
  using SparseForwardDataFlowAnalysis::SparseForwardDataFlowAnalysis;
  friend class SecretnessAnalysisDependent<SymbolAnalysis>;

  void setToEntryState(SymbolLattice *lattice) override {
    propagateIfChanged(lattice, lattice->join(SymbolState()));
  }

  LogicalResult visitOperation(Operation *op,
                               ArrayRef<const SymbolLattice *> operands,
                               ArrayRef<SymbolLattice *> results) override;

  void visitExternalCall(CallOpInterface call,
                         ArrayRef<const SymbolLattice *> argumentLattices,
                         ArrayRef<SymbolLattice *> resultLattices) override;

  void propagateIfChangedWrapper(AnalysisState *state, ChangeResult changed) {
    propagateIfChanged(state, changed);
  }

 private:
  int counter = 0;
};

}  // namespace heir
}  // namespace mlir

#endif  // LIB_ANALYSIS_SYMBOLANALYSIS_SYMBOLANALYSIS_H_
