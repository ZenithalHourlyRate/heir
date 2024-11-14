#include "lib/Analysis/MulDepthAnalysis/MulDepthAnalysis.h"
#include "lib/Dialect/Secret/IR/SecretOps.h"
#include "lib/Transforms/AnnotateSecretManagement/Passes.h"
#include "llvm/include/llvm/ADT/TypeSwitch.h"  // from @llvm-project
#include "llvm/include/llvm/Support/Debug.h"   // from @llvm-project
#include "mlir/include/mlir/Analysis/DataFlow/ConstantPropagationAnalysis.h"  // from @llvm-project
#include "mlir/include/mlir/Analysis/DataFlow/DeadCodeAnalysis.h"  // from @llvm-project
#include "mlir/include/mlir/Dialect/Arith/IR/Arith.h"   // from @llvm-project
#include "mlir/include/mlir/Dialect/Func/IR/FuncOps.h"  // from @llvm-project
#include "mlir/include/mlir/IR/Iterators.h"             // from @llvm-project
#include "mlir/include/mlir/Transforms/GreedyPatternRewriteDriver.h"  // from @llvm-project
#include "mlir/include/mlir/Transforms/Passes.h"  // from @llvm-project

#define DEBUG_TYPE "annotate-secret-management"

namespace mlir {
namespace heir {

#define GEN_PASS_DEF_ANNOTATESECRETMANAGEMENT
#include "lib/Transforms/AnnotateSecretManagement/Passes.h.inc"

struct AnnotateSecretManagement
    : impl::AnnotateSecretManagementBase<AnnotateSecretManagement> {
  using AnnotateSecretManagementBase::AnnotateSecretManagementBase;

  void runOnOperation() override {
    ModuleOp module = getOperation();
    OpBuilder builder(module);

    getOperation()->walk<WalkOrder::PreOrder>([&](secret::GenericOp genericOp) {
      // Analyse the operations to find the MulDepth
      DataFlowSolver solver;
      solver.load<dataflow::DeadCodeAnalysis>();
      solver.load<dataflow::SparseConstantPropagation>();
      solver.load<MulDepthAnalysis>();
      if (failed(solver.initializeAndRun(genericOp))) {
        getOperation()->emitOpError() << "Failed to run the analysis.\n";
        signalPassFailure();
        return;
      }

      int64_t maxMulDepth = 0;
      // walk the operations to find the max MulDepth
      genericOp.getBody()->walk([&](Operation *op) {
        // if the lengths of the operands is 0, then return
        if (op->getNumResults() == 0) return WalkResult::advance();
        const MulDepthLattice *resultLattice =
            solver.lookupState<MulDepthLattice>(op->getResult(0));
        if (resultLattice->getValue().isInitialized()) {
          maxMulDepth =
              std::max(maxMulDepth, resultLattice->getValue().getValue());
        }
        return WalkResult::advance();
      });

      genericOp->setAttr("depth", builder.getIntegerAttr(
                                      builder.getIntegerType(64), maxMulDepth));

      genericOp.getBody()->walk<WalkOrder::PreOrder>([&](Operation *op) {
        // if the lengths of the operands is 0, then return
        // if (op->getNumResults() == 0) return WalkResult::advance();

        llvm::TypeSwitch<Operation &>(*op).Case<arith::MulIOp, arith::MulFOp>(
            [&](auto arithOp) {
              auto &mulDepthResult =
                  solver.lookupState<MulDepthLattice>(op->getResult(0))
                      ->getValue();
              auto &mulDepthLhs =
                  solver.lookupState<MulDepthLattice>(op->getOperand(0))
                      ->getValue();
              auto &mulDepthRhs =
                  solver.lookupState<MulDepthLattice>(op->getOperand(1))
                      ->getValue();

              auto mulDepthResultValue = 0;
              auto mulDepthLhsValue = 0;
              auto mulDepthRhsValue = 0;

              if (mulDepthResult.isInitialized()) {
                mulDepthResultValue = mulDepthResult.getValue();
              }
              if (mulDepthLhs.isInitialized()) {
                mulDepthLhsValue = mulDepthLhs.getValue();
              }
              if (mulDepthRhs.isInitialized()) {
                mulDepthRhsValue = mulDepthRhs.getValue();
              }

              op->setAttr("depth",
                          builder.getIntegerAttr(builder.getIntegerType(64),
                                                 mulDepthResultValue));
              op->setAttr("lhs",
                          builder.getIntegerAttr(builder.getIntegerType(64),
                                                 mulDepthLhsValue));
              op->setAttr("rhs",
                          builder.getIntegerAttr(builder.getIntegerType(64),
                                                 mulDepthRhsValue));
              op->setAttr("relin", builder.getIntegerAttr(
                                       builder.getIntegerType(64), 3));
            });
      });
    });
  }
};

}  // namespace heir
}  // namespace mlir
