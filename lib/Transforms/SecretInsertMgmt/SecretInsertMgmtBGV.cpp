#include <utility>

#include "lib/Analysis/LevelAnalysis/LevelAnalysis.h"
#include "lib/Analysis/MulResultAnalysis/MulResultAnalysis.h"
#include "lib/Analysis/NoisePropagation/NoisePropagationAnalysis.h"
#include "lib/Analysis/NoisePropagation/ParamAnalysis.h"
#include "lib/Analysis/NoisePropagation/Params.h"
#include "lib/Analysis/SecretnessAnalysis/SecretnessAnalysis.h"
#include "lib/Dialect/Mgmt/IR/MgmtAttributes.h"
#include "lib/Dialect/Mgmt/IR/MgmtOps.h"
#include "lib/Dialect/Mgmt/Transforms/AnnotateMgmt.h"
#include "lib/Dialect/Mgmt/Transforms/Passes.h"
#include "lib/Dialect/Secret/IR/SecretOps.h"
#include "lib/Transforms/SecretInsertMgmt/Passes.h"
#include "lib/Transforms/SecretInsertMgmt/SecretInsertMgmtPatterns.h"
#include "llvm/include/llvm/ADT/TypeSwitch.h"  // from @llvm-project
#include "mlir/include/mlir/Analysis/DataFlow/ConstantPropagationAnalysis.h"  // from @llvm-project
#include "mlir/include/mlir/Analysis/DataFlow/DeadCodeAnalysis.h"  // from @llvm-project
#include "mlir/include/mlir/Analysis/DataFlowFramework.h"  // from @llvm-project
#include "mlir/include/mlir/Dialect/Arith/IR/Arith.h"      // from @llvm-project
#include "mlir/include/mlir/Dialect/Func/IR/FuncOps.h"     // from @llvm-project
#include "mlir/include/mlir/Dialect/Tensor/IR/Tensor.h"    // from @llvm-project
#include "mlir/include/mlir/IR/Iterators.h"                // from @llvm-project
#include "mlir/include/mlir/IR/PatternMatch.h"             // from @llvm-project
#include "mlir/include/mlir/Pass/PassManager.h"            // from @llvm-project
#include "mlir/include/mlir/Support/LLVM.h"                // from @llvm-project
#include "mlir/include/mlir/Transforms/Passes.h"           // from @llvm-project
#include "mlir/include/mlir/Transforms/WalkPatternRewriteDriver.h"  // from @llvm-project

namespace mlir {
namespace heir {

#define GEN_PASS_DEF_SECRETINSERTMGMTBGV
#include "lib/Transforms/SecretInsertMgmt/Passes.h.inc"

struct SecretInsertMgmtBGV
    : impl::SecretInsertMgmtBGVBase<SecretInsertMgmtBGV> {
  using SecretInsertMgmtBGVBase::SecretInsertMgmtBGVBase;

  void annotateBound() {
    DataFlowSolver solver;
    solver.load<dataflow::DeadCodeAnalysis>();
    solver.load<dataflow::SparseConstantPropagation>();
    solver.load<ParamAnalysis>();
    solver.load<NoiseAnalysis>();
    if (failed(solver.initializeAndRun(getOperation()))) {
      getOperation()->emitOpError() << "Failed to run the analysis.\n";
      signalPassFailure();
      return;
    }

    auto firstModSize = 0;
    // for level i, the biggest gap observed.
    std::map<int, double> levelToGap;

    auto updateLevelToGap = [&](int level, double gap) {
      if (levelToGap.count(level) == 0) {
        levelToGap[level] = gap;
      } else {
        levelToGap[level] = std::max(levelToGap.at(level), gap);
      }
    };

    getOperation()->walk<WalkOrder::PreOrder>([&](secret::GenericOp genericOp) {
      for (auto blockArg : genericOp.getBody()->getArguments()) {
        auto &param = solver.lookupState<ParamLattice>(blockArg)->getValue();
        auto &noise = solver.lookupState<NoiseLattice>(blockArg)->getValue();
        if (!param.isInitialized() || !noise.isInitialized()) {
          continue;
        }
        auto bound = noise.toBound(param.getLocalParam());
        genericOp.setArgAttr(blockArg.getArgNumber(), "bound",
                             StringAttr::get(&getContext(), bound));
      }

      genericOp.getBody()->walk<WalkOrder::PreOrder>([&](Operation *op) {
        if (op->getNumResults() == 0) {
          return;
        }
        auto &param =
            solver.lookupState<ParamLattice>(op->getResult(0))->getValue();
        auto &noise =
            solver.lookupState<NoiseLattice>(op->getResult(0))->getValue();
        if (!noise.isInitialized()) {
          return;
        }

        auto level = cast<mgmt::MgmtAttr>(
                         op->getAttr(mgmt::MgmtDialect::kArgMgmtAttrName))
                         .getLevel();
        auto bound = noise.toBound(param.getLocalParam());
        op->setAttr("bound", StringAttr::get(&getContext(), bound));

        // scalingModPart
        if (isa<mgmt::ModReduceOp>(op)) {
          auto upperLevelNoise =
              solver.lookupState<NoiseLattice>(op->getOperand(0))->getValue();
          auto upperLevelParam =
              solver.lookupState<ParamLattice>(op->getOperand(0))->getValue();
          auto upperLevelBound =
              upperLevelNoise.toBound(upperLevelParam.getLocalParam());

          // FIXME: stod?
          updateLevelToGap(level,
                           std::stod(upperLevelBound) - std::stod(bound));
        }

        // firstModPart
        if (level == 0) {
          firstModSize =
              std::max(firstModSize, 1 + int(ceil(std::stod(bound))));
        }
      });
    });

    auto scalingModSize = 0;

    auto maxLevel = levelToGap.size() + 1;
    auto qiSize = std::vector<int>(maxLevel, 0);
    qiSize[0] = firstModSize;

    getOperation()->walk<WalkOrder::PreOrder>([&](secret::GenericOp genericOp) {
      for (auto &[level, gap] : levelToGap) {
        scalingModSize = std::max(scalingModSize, int(ceil(gap)));
        genericOp->setAttr(
            "gap_" + std::to_string(level),
            StringAttr::get(&getContext(), std::to_string(int(ceil(gap)))));
        qiSize[level + 1] = int(ceil(gap));
      }

      auto *funcOp = genericOp->getParentOp();
      // TODO: better firstModSize selection
      funcOp->setAttr("firstModSize",
                      IntegerAttr::get(IntegerType::get(&getContext(), 64),
                                       scalingModSize));
      funcOp->setAttr("scalingModSize",
                      IntegerAttr::get(IntegerType::get(&getContext(), 64),
                                       scalingModSize));
    });

    // auto concreteParam = SchemeParamsFactory::genConcreteParam(
    //     maxLevel - 1, 0, 2, 65537, qiSize, 2);
    // LLVM_DEBUG(llvm::dbgs()
    //            << "Concrete scheme param " << concreteParam << "\n");
  }

  void annotateSchemeParams() {
    DataFlowSolver solver;
    solver.load<dataflow::DeadCodeAnalysis>();
    solver.load<dataflow::SparseConstantPropagation>();
    solver.load<ParamAnalysis>();
    if (failed(solver.initializeAndRun(getOperation()))) {
      getOperation()->emitOpError() << "Failed to run the analysis.\n";
      signalPassFailure();
      return;
    }

    auto getIntegerAttr = [&](int64_t n) {
      return IntegerAttr::get(IntegerType::get(&getContext(), 64), n);
    };
    auto getStringAttr = [&](const std::string &str) {
      return StringAttr::get(&getContext(), str);
    };

    getOperation()->walk<WalkOrder::PreOrder>([&](secret::GenericOp genericOp) {
      auto blockArg0 = genericOp.getBody()->getArgument(0);
      auto &param = solver.lookupState<ParamLattice>(blockArg0)->getValue();
      if (!param.isInitialized()) {
        return;
      }
      auto *schemeParam = param.getLocalParam().getSchemeParam();

      // FIXME: better way to get funcOp
      auto *funcOp = genericOp->getParentOp();
      // TODO: recalculate N, thus P
      funcOp->setAttr("ringDim", getIntegerAttr(schemeParam->n));
      funcOp->setAttr("multiplicativeDepth", getIntegerAttr(schemeParam->L));
      funcOp->setAttr("plaintextModulus", getIntegerAttr(schemeParam->t));
      funcOp->setAttr("maxRelinSkDeg",
                      getIntegerAttr(schemeParam->maxRelinSkDeg));
      funcOp->setAttr("scalingTechnique", getStringAttr("FIXEDMANUAL"));
      // TODO: acquire scalingModSize from noise analysis
      // funcOp->setAttr("scalingModSize", getIntegerAttr(schemeParam->qi[0]));
      funcOp->setAttr("keySwitchTechnique",
                      getStringAttr(schemeParam->dnum != 0 ? "HYBRID" : "BV"));
      funcOp->setAttr("digitSize", getIntegerAttr(schemeParam->digitSize));
      funcOp->setAttr("numLargeDigits", getIntegerAttr(schemeParam->dnum));
    });
  }

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

    RewritePatternSet patternsMultModReduce(&getContext());
    patternsMultModReduce.add<ModReduceBefore<arith::MulIOp>>(
        &getContext(), /*isMul*/ true, includeFirstMul, getOperation(),
        &solver);
    // tensor::ExtractOp = mulConst + rotate
    patternsMultModReduce.add<ModReduceBefore<tensor::ExtractOp>>(
        &getContext(), /*isMul*/ true, includeFirstMul, getOperation(),
        &solver);
    // isMul = true and includeFirstMul = false here
    // as before yield we want mulResult to be mod reduced
    patternsMultModReduce.add<ModReduceBefore<secret::YieldOp>>(
        &getContext(), /*isMul*/ true, /*includeFirstMul*/ false,
        getOperation(), &solver);
    (void)walkAndApplyPatterns(getOperation(),
                               std::move(patternsMultModReduce));

    // when other binary op operands level mismatch
    // includeFirstMul not used for these ops
    RewritePatternSet patternsAddModReduce(&getContext());
    patternsAddModReduce.add<ModReduceBefore<arith::AddIOp>>(
        &getContext(), /*isMul*/ false, /*includeFirstMul*/ false,
        getOperation(), &solver);
    patternsAddModReduce.add<ModReduceBefore<arith::SubIOp>>(
        &getContext(), /*isMul*/ false, /*includeFirstMul*/ false,
        getOperation(), &solver);
    (void)walkAndApplyPatterns(getOperation(), std::move(patternsAddModReduce));

    // call CSE here because there may be redundant mod reduce
    // one Value may get mod reduced multiple times in
    // multiple Uses
    //
    // also run annotate-mgmt for lowering
    OpPassManager pipeline("builtin.module");
    pipeline.addPass(createCSEPass());
    pipeline.addPass(mgmt::createAnnotateMgmt());
    (void)runPipeline(pipeline, getOperation());

    // custom
    annotateBound();
    annotateSchemeParams();
  }
};

}  // namespace heir
}  // namespace mlir
