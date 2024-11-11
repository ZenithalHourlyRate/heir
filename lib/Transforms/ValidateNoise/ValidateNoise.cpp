#include "lib/Transforms/ValidateNoise/ValidateNoise.h"

#include "lib/Analysis/NoisePropagation/NoisePropagationAnalysis.h"
#include "lib/Analysis/NoisePropagation/Variance.h"
#include "lib/Dialect/LWE/IR/LWETypes.h"
#include "lib/Interfaces/NoiseInterfaces.h"
#include "llvm/include/llvm/Support/Debug.h"  // from @llvm-project
#include "mlir/include/mlir/Analysis/DataFlow/ConstantPropagationAnalysis.h"  // from @llvm-project
#include "mlir/include/mlir/Analysis/DataFlow/DeadCodeAnalysis.h"  // from @llvm-project
#include "mlir/include/mlir/Analysis/DataFlow/IntegerRangeAnalysis.h"  // from @llvm-projectject
#include "mlir/include/mlir/Analysis/DataFlowFramework.h"  // from @llvm-project
#include "mlir/include/mlir/IR/Visitors.h"                 // from @llvm-project
#include "mlir/include/mlir/Pass/Pass.h"                   // from @llvm-project

#define DEBUG_TYPE "ValidateNoise"

namespace mlir {
namespace heir {

#define GEN_PASS_DEF_VALIDATENOISE
#include "lib/Transforms/ValidateNoise/ValidateNoise.h.inc"

struct ValidateNoise : impl::ValidateNoiseBase<ValidateNoise> {
  using ValidateNoiseBase::ValidateNoiseBase;

  void runOnOperation() override {
    auto *module = getOperation();

    DataFlowSolver solver;
    // The dataflow solver needs DeadCodeAnalysis and SparseConstantPropagation
    // to run pretty much any data flow analysis, see
    // https://discourse.llvm.org/t/mlir-dead-code-analysis/67568/8
    solver.load<dataflow::DeadCodeAnalysis>();
    solver.load<dataflow::SparseConstantPropagation>();
    solver.load<NoiseStatesAnalysis>();
    if (failed(solver.initializeAndRun(module))) {
      getOperation()->emitOpError() << "Failed to run the analysis.\n";
      signalPassFailure();
      return;
    }

    int valueName = 0;
    DenseMap<Value, std::string> valueNameMap;

    auto result = module->walk([&](Operation *op) {
      for (OpResult result : op->getResults()) {
        const VarianceStatesLattice *opRange =
            solver.lookupState<VarianceStatesLattice>(result);
        if (!opRange) {
          return WalkResult::interrupt();
        }

        valueNameMap[result] = std::to_string(valueName++);
#if 1
        LLVM_DEBUG(llvm::dbgs() << opRange->getValue().toDOTNode(valueNameMap)
                                << opRange->getValue().toDOTEdge(valueNameMap));
#endif
#if 0
        auto &vss = opRange->getValue();
        auto params = vss.reachable();
        if (params.size() != 0) {
          LLVM_DEBUG(llvm::dbgs() << "Reachable params for "
                                  << valueNameMap.at(result) << "\n");
          for (auto &p : params) {
            LLVM_DEBUG(llvm::dbgs() << p << "\n");
          }
        } else {
          LLVM_DEBUG(llvm::dbgs() << "No reachable params for "
                                  << valueNameMap.at(result) << "\n");
        }
#endif
      }
      return WalkResult::advance();
    });
  }
};

}  // namespace heir
}  // namespace mlir
