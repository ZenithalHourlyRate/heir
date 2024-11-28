#include "lib/Analysis/DimensionAnalysis/DimensionAnalysis.h"
#include "lib/Analysis/LevelAnalysis/LevelAnalysis.h"
#include "lib/Analysis/MulResultAnalysis/MulResultAnalysis.h"
#include "lib/Analysis/SecretnessAnalysis/SecretnessAnalysis.h"
#include "lib/Dialect/Secret/IR/SecretOps.h"
#include "lib/Transforms/SecretWithMgmt/Passes.h"
#include "lib/Transforms/SecretWithMgmt/SecretWithMgmtPatterns.h"
#include "llvm/include/llvm/ADT/TypeSwitch.h"  // from @llvm-project
#include "llvm/include/llvm/Support/Debug.h"   // from @llvm-project
#include "mlir/include/mlir/Analysis/DataFlow/ConstantPropagationAnalysis.h"  // from @llvm-project
#include "mlir/include/mlir/Analysis/DataFlow/DeadCodeAnalysis.h"  // from @llvm-project
#include "mlir/include/mlir/Dialect/Arith/IR/Arith.h"    // from @llvm-project
#include "mlir/include/mlir/Dialect/Func/IR/FuncOps.h"   // from @llvm-project
#include "mlir/include/mlir/Dialect/Tensor/IR/Tensor.h"  // from @llvm-project
#include "mlir/include/mlir/IR/Iterators.h"              // from @llvm-project
#include "mlir/include/mlir/Transforms/Passes.h"         // from @llvm-project
#include "mlir/include/mlir/Transforms/WalkPatternRewriteDriver.h"  // from @llvm-project

#define DEBUG_TYPE "secret-with-mgmt-bgv"

namespace mlir {
namespace heir {

#define GEN_PASS_DEF_SECRETWITHMGMTBGV
#include "lib/Transforms/SecretWithMgmt/Passes.h.inc"

struct SecretWithMgmtBGV : impl::SecretWithMgmtBGVBase<SecretWithMgmtBGV> {
  using SecretWithMgmtBGVBase::SecretWithMgmtBGVBase;

  void runOnOperation() override {
    DataFlowSolver solver;
    solver.load<dataflow::DeadCodeAnalysis>();
    solver.load<dataflow::SparseConstantPropagation>();
    solver.load<SecretnessAnalysis>();
    solver.load<MulResultAnalysis>();
    solver.load<LevelAnalysis>();

    if (failed(solver.initializeAndRun(getOperation()))) {
      getOperation()->emitOpError() << "Failed to run the analysis.\n";
      signalPassFailure();
      return;
    }

    RewritePatternSet patternsRelinearize(&getContext());
    patternsRelinearize.add<MultRelinearize<arith::MulIOp>>(
        &getContext(), getOperation(), &solver);
    (void)walkAndApplyPatterns(getOperation(), std::move(patternsRelinearize));

    // only after relinearize we can get the correct dimension
    // NOTE: for lazy relinearize, need to uninitialize the DimensionLattice...
    // otherwise DimensionState::join may not work correctly
    // if we are reusing a solver
    solver.load<DimensionAnalysis>();

    RewritePatternSet patternsMultModReduce(&getContext());
    patternsMultModReduce.add<ModReduceBefore<arith::MulIOp>>(
        &getContext(), /*isMul*/ true, includeFirst, getOperation(), &solver);
    // tensor::ExtractOp = mulConst + rotate
    patternsMultModReduce.add<ModReduceBefore<tensor::ExtractOp>>(
        &getContext(), /*isMul*/ true, includeFirst, getOperation(), &solver);
    // isMul = true and includeFirst = false here
    // as before yield we want mulResult to be mod reduced
    patternsMultModReduce.add<ModReduceBefore<secret::YieldOp>>(
        &getContext(), /*isMul*/ true, /*includeFirst*/ false, getOperation(),
        &solver);
    (void)walkAndApplyPatterns(getOperation(),
                               std::move(patternsMultModReduce));

    // when other binary op operands level mismatch
    // includeFirst not used for these ops
    RewritePatternSet patternsAddModReduce(&getContext());
    patternsAddModReduce.add<ModReduceBefore<arith::AddIOp>>(
        &getContext(), /*isMul*/ false, /*includeFirst*/ false, getOperation(),
        &solver);
    patternsAddModReduce.add<ModReduceBefore<arith::SubIOp>>(
        &getContext(), /*isMul*/ false, /*includeFirst*/ false, getOperation(),
        &solver);
    (void)walkAndApplyPatterns(getOperation(), std::move(patternsAddModReduce));

    // call CSE here because there may be redundant mod reduce
    // one Value may get mod reduced multiple times in
    // multiple Uses
    OpPassManager csePipeline("builtin.module");
    csePipeline.addPass(createCSEPass());
    (void)runPipeline(csePipeline, getOperation());

    // re-run analysis after CSE
    if (failed(solver.initializeAndRun(getOperation()))) {
      getOperation()->emitOpError() << "Failed to run the analysis.\n";
      signalPassFailure();
      return;
    }

    annotateLevel(getOperation(), &solver);
    annotateDimension(getOperation(), &solver);
  }
};

}  // namespace heir
}  // namespace mlir
