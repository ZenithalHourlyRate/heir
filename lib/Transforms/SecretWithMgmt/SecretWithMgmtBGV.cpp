#include "lib/Analysis/NoisePropagation/NoisePropagationAnalysis.h"
#include "lib/Analysis/NoisePropagation/ParamAnalysis.h"
#include "lib/Analysis/SecretnessAnalysis/SecretnessAnalysis.h"
#include "lib/Dialect/Mgmt/IR/MgmtOps.h"
#include "lib/Dialect/Secret/IR/SecretOps.h"
#include "lib/Dialect/TensorExt/IR/TensorExtOps.h"
#include "lib/Transforms/SecretWithMgmt/Passes.h"
#include "llvm/include/llvm/ADT/TypeSwitch.h"  // from @llvm-project
#include "llvm/include/llvm/Support/Debug.h"   // from @llvm-project
#include "mlir/include/mlir/Analysis/DataFlow/ConstantPropagationAnalysis.h"  // from @llvm-project
#include "mlir/include/mlir/Analysis/DataFlow/DeadCodeAnalysis.h"  // from @llvm-project
#include "mlir/include/mlir/Dialect/Arith/IR/Arith.h"    // from @llvm-project
#include "mlir/include/mlir/Dialect/Func/IR/FuncOps.h"   // from @llvm-project
#include "mlir/include/mlir/Dialect/Tensor/IR/Tensor.h"  // from @llvm-project
#include "mlir/include/mlir/IR/Iterators.h"              // from @llvm-project
#include "mlir/include/mlir/Transforms/GreedyPatternRewriteDriver.h"  // from @llvm-project
#include "mlir/include/mlir/Transforms/Passes.h"  // from @llvm-project

#define DEBUG_TYPE "secret-with-mgmt-bgv"

namespace mlir {
namespace heir {

#define GEN_PASS_DEF_SECRETWITHMGMTBGV
#include "lib/Transforms/SecretWithMgmt/Passes.h.inc"

struct SecretWithMgmtBGV : impl::SecretWithMgmtBGVBase<SecretWithMgmtBGV> {
  using SecretWithMgmtBGVBase::SecretWithMgmtBGVBase;

  void multiplicationAlwaysRelinearize() {
    // TODO: handle tensor::ExtractOp
    OpBuilder b(&getContext());
    getOperation()->walk<WalkOrder::PreOrder>([&](secret::GenericOp genericOp) {
      genericOp.getBody()->walk<WalkOrder::PreOrder>([&](Operation *op) {
        llvm::TypeSwitch<Operation &>(*op).Case<arith::MulIOp>([&](auto mulOp) {
          b.setInsertionPointAfter(mulOp);
          Value result = mulOp.getResult();
          auto relinearized =
              b.create<mgmt::RelinearizeOp>(mulOp->getLoc(), result);
          result.replaceAllUsesExcept(relinearized, {relinearized});
        });
      });
    });
  }

  void multiplicationAlwaysModReduceBefore(bool includeFirst) {
    DataFlowSolver solver;
    solver.load<dataflow::DeadCodeAnalysis>();
    solver.load<dataflow::SparseConstantPropagation>();
    solver.load<SecretnessAnalysis>();
    if (failed(solver.initializeAndRun(getOperation()))) {
      getOperation()->emitOpError() << "Failed to run the analysis.\n";
      signalPassFailure();
      return;
    }

    auto getSecretness = [&](Value value) {
      const auto *lattice = solver.lookupState<SecretnessLattice>(value);
      if (!lattice) {
        // newly created value does not have lattice
        return true;
      }
      auto &secretness =
          solver.lookupState<SecretnessLattice>(value)->getValue();
      if (secretness.isInitialized()) {
        return secretness.getSecretness();
      }
      // if not initialized assume secret
      return true;
    };

    DenseMap<Value, int> levelMap;
    DenseMap<Value, bool> mulMap;  // check a Value is a mul result or not

    OpBuilder b(&getContext());
    getOperation()->walk<WalkOrder::PreOrder>([&](secret::GenericOp genericOp) {
      for (auto blockArg : genericOp.getBody()->getArguments()) {
        levelMap[blockArg] = 0;
        mulMap[blockArg] = false;
      }

      genericOp.getBody()->walk<WalkOrder::PreOrder>([&](Operation *op) {
        auto levelResult = 0;
        for (auto operand : op->getOperands()) {
          if (levelMap.count(operand) == 0) {
            continue;
          }
          auto levelOperand = levelMap.at(operand);
          levelResult = std::max(levelResult, levelOperand);
        }

        bool operandsMul = false;
        for (auto operand : op->getOperands()) {
          if (mulMap.count(operand) == 0) {
            continue;
          }
          operandsMul |= mulMap.at(operand);
        }

        llvm::TypeSwitch<Operation &>(*op)
            .Case<arith::MulIOp, secret::YieldOp, tensor::ExtractOp>(
                [&](auto mulOp) {
                  // mod reduced
                  if ((isa<arith::MulIOp>(mulOp) ||
                       isa<tensor::ExtractOp>(mulOp)) &&
                      (includeFirst || operandsMul)) {
                    levelResult += 1;
                  }
                  if (isa<secret::YieldOp>(mulOp) && operandsMul) {
                    // mod reduce before yield if muled
                    levelResult += 1;
                  }
                  // avoid yield op
                  if (mulOp->getNumResults() != 0) {
                    levelMap[mulOp->getResult(0)] = levelResult;
                    mulMap[mulOp->getResult(0)] = true;
                  }

                  for (auto operand : mulOp->getOperands()) {
                    auto secretness = getSecretness(operand);
                    if (!secretness) {
                      continue;
                    }
                    b.setInsertionPoint(mulOp);
                    Value managed = operand;
                    for (auto i = 0; i != levelResult - levelMap.at(operand);
                         ++i) {
                      managed =
                          b.create<mgmt::ModReduceOp>(mulOp->getLoc(), managed);
                      levelMap[managed] = levelMap.at(operand) + i + 1;
                      mulMap[managed] = mulMap.at(operand);
                    }
                    mulOp->replaceUsesOfWith(operand, managed);
                  }
                })
            .Default([&](auto &op) {
              for (auto result : op.getResults()) {
                levelMap[result] = levelResult;
                mulMap[result] = operandsMul;
              }
            });
      });
    });
  }

  // implicitly done now
  void rotationAlwaysRelinearize() {
    OpBuilder b(&getContext());
    getOperation()->walk<WalkOrder::PreOrder>([&](secret::GenericOp genericOp) {
      genericOp.getBody()->walk<WalkOrder::PreOrder>([&](Operation *op) {
        llvm::TypeSwitch<Operation &>(*op).Case<tensor_ext::RotateOp>(
            [&](auto rotateOp) {
              b.setInsertionPointAfter(rotateOp);
              Value result = rotateOp.getResult();
              auto relinearized =
                  b.create<mgmt::RelinearizeOp>(rotateOp->getLoc(), result);
              result.replaceAllUsesExcept(relinearized, {relinearized});
            });
      });
    });
  }

  void alwaysModreduceWhenLevelMismatch() {
    DataFlowSolver solver;
    solver.load<dataflow::DeadCodeAnalysis>();
    solver.load<dataflow::SparseConstantPropagation>();
    solver.load<SecretnessAnalysis>();
    if (failed(solver.initializeAndRun(getOperation()))) {
      getOperation()->emitOpError() << "Failed to run the analysis.\n";
      signalPassFailure();
      return;
    }

    auto getSecretness = [&](Value value) {
      auto &secretness =
          solver.lookupState<SecretnessLattice>(value)->getValue();
      return secretness;
    };

    DenseMap<Value, int> levelMap;

    getOperation()->walk<WalkOrder::PreOrder>([&](secret::GenericOp genericOp) {
      for (auto blockArg : genericOp.getBody()->getArguments()) {
        levelMap[blockArg] = 0;
      }

      genericOp.getBody()->walk<WalkOrder::PreOrder>([&](Operation *op) {
        auto levelResult = 0;
        for (auto operand : op->getOperands()) {
          if (levelMap.count(operand) == 0) {
            continue;
          }
          auto levelOperand = levelMap.at(operand);
          levelResult = std::max(levelResult, levelOperand);
        }
        for (auto result : op->getResults()) {
          levelMap[result] = levelResult;
          if (mlir::isa<mgmt::ModReduceOp>(op)) {
            levelMap[result] = levelResult + 1;
          }
        }
      });
    });

    getOperation()->walk<WalkOrder::PreOrder>([&](secret::GenericOp genericOp) {
      genericOp.getBody()->walk<WalkOrder::PreOrder>([&](Operation *op) {
        ImplicitLocOpBuilder b(op->getLoc(), op);
        llvm::TypeSwitch<Operation &>(*op).Case<arith::MulIOp, arith::AddIOp>(
            [&](auto arithOp) {
              auto levelResult = levelMap.at(op->getResult(0));
              auto secretnessResult = getSecretness(op->getResult(0));

              if (secretnessResult.isInitialized() &&
                  !secretnessResult.getSecretness()) {
                return;
              }

              for (auto operand : op->getOperands()) {
                auto secretnessOperand = getSecretness(operand);

                // skip mod reduce if operand is not secret
                if (!secretnessOperand.isInitialized() ||
                    !secretnessOperand.getSecretness()) {
                  continue;
                }

                auto levelOperand = levelMap.at(operand);
                if (levelOperand < levelResult) {
                  Value managed = operand;
                  for (auto i = 0; i != levelResult - levelOperand; ++i) {
                    managed = b.create<mgmt::ModReduceOp>(managed);
                    levelMap[managed] = levelOperand + i + 1;
                  }
                  op->replaceUsesOfWith(operand, managed);
                }
              }
            });
      });
    });
  }

  int annotateLevel() {
    // we use 0 to L+1 for now; finally reverse to get L+1 to 0
    DenseMap<Value, int> levelMap;

    getOperation()->walk<WalkOrder::PreOrder>([&](secret::GenericOp genericOp) {
      for (auto blockArg : genericOp.getBody()->getArguments()) {
        levelMap[blockArg] = 0;
      }

      genericOp.getBody()->walk<WalkOrder::PreOrder>([&](Operation *op) {
        auto levelResult = 0;
        for (auto operand : op->getOperands()) {
          if (levelMap.count(operand) == 0) {
            continue;
          }
          auto levelOperand = levelMap.at(operand);
          levelResult = std::max(levelResult, levelOperand);
        }
        for (auto result : op->getResults()) {
          levelMap[result] = levelResult;
          if (mlir::isa<mgmt::ModReduceOp>(op)) {
            levelMap[result] = levelResult + 1;
          }
        }
      });
    });

    // reverse the level
    int maxLevel = 0;
    for (auto &[value, level] : levelMap) {
      maxLevel = std::max(maxLevel, level);
    }
    for (auto &[value, level] : levelMap) {
      level = maxLevel - level;
    }

    getOperation()->walk<WalkOrder::PreOrder>([&](secret::GenericOp genericOp) {
      for (auto i = 0; i != genericOp.getBody()->getNumArguments(); ++i) {
        auto blockArg = genericOp.getBody()->getArgument(i);
        auto level = levelMap.at(blockArg);
        genericOp.setArgAttr(
            i, "level",
            IntegerAttr::get(IntegerType::get(&getContext(), 64), level));
      }

      genericOp.getBody()->walk<WalkOrder::PreOrder>([&](Operation *op) {
        if (op->getNumResults() == 0) {
          return;
        }
        auto result = op->getResult(0);
        op->setAttr("level",
                    IntegerAttr::get(IntegerType::get(&getContext(), 64),
                                     levelMap.at(result)));
      });
    });

    return maxLevel;
  }

  void annotatePlaintextLevel(int maxLevel) {
    DataFlowSolver solver;
    solver.load<dataflow::DeadCodeAnalysis>();
    solver.load<dataflow::SparseConstantPropagation>();
    // NOTE: MulDepthAnalysis works because of
    // multiplicationAlwaysRelinearizeAndModReduce
    // where modreduceop has the same mulDepthLattice.
    solver.load<SecretnessAnalysis>();
    if (failed(solver.initializeAndRun(getOperation()))) {
      getOperation()->emitOpError() << "Failed to run the analysis.\n";
      signalPassFailure();
      return;
    }

    getOperation()->walk<WalkOrder::PreOrder>([&](secret::GenericOp genericOp) {
      genericOp.getBody()->walk<WalkOrder::PreOrder>([&](Operation *op) {
        if (op->getNumResults() == 0) {
          return;
        }
        auto level = cast<IntegerAttr>(op->getAttr("level")).getInt();
        for (auto operand : op->getOperands()) {
          auto secretness =
              solver.lookupState<SecretnessLattice>(operand)->getValue();
          // is plaintext
          if (secretness.isInitialized() && !secretness.getSecretness()) {
            operand.getDefiningOp()->setAttr(
                "level_pt",
                IntegerAttr::get(IntegerType::get(&getContext(), 64), level));
            operand.getDefiningOp()->setAttr(
                "level_scheme",
                IntegerAttr::get(IntegerType::get(&getContext(), 64),
                                 maxLevel));
          }
        }
      });
    });
  }

  void annotateDimension() {
    DenseMap<Value, int> dimensionMap;

    getOperation()->walk<WalkOrder::PreOrder>([&](secret::GenericOp genericOp) {
      for (auto blockArg : genericOp.getBody()->getArguments()) {
        auto dimension = 2;
        dimensionMap[blockArg] = 2;
        genericOp.setArgAttr(
            blockArg.getArgNumber(), "dimension",
            IntegerAttr::get(IntegerType::get(&getContext(), 64), dimension));
      }

      genericOp.getBody()->walk<WalkOrder::PreOrder>([&](Operation *op) {
        if (op->getNumResults() == 0) {
          return;
        }
        auto resultDimension = 0;
        for (auto operand : op->getOperands()) {
          auto operandDimension = 2;
          if (dimensionMap.count(operand) != 0) {
            operandDimension = dimensionMap.at(operand);
          }
          if (isa<arith::MulIOp>(op)) {
            resultDimension += operandDimension;
          } else {
            resultDimension = std::max(resultDimension, operandDimension);
          }
        }
        if (isa<arith::MulIOp>(op)) {
          resultDimension -= 1;
        }
        if (isa<mgmt::RelinearizeOp>(op)) {
          resultDimension = 2;
        }
        dimensionMap[op->getResult(0)] = resultDimension;
        op->setAttr("dimension",
                    IntegerAttr::get(IntegerType::get(&getContext(), 64),
                                     resultDimension));
      });
    });
  }

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
        auto bound = noise.toBound(param.getLocalParam());
        op->setAttr("bound", StringAttr::get(&getContext(), bound));
      });
    });
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
      funcOp->setAttr("ringDim", getIntegerAttr(schemeParam->n));
      funcOp->setAttr("multiplicativeDepth", getIntegerAttr(schemeParam->L));
      funcOp->setAttr("plaintextModulus", getIntegerAttr(schemeParam->t));
      funcOp->setAttr("maxRelinSkDeg",
                      getIntegerAttr(schemeParam->maxRelinSkDeg));
      funcOp->setAttr("scalingModSize", getIntegerAttr(schemeParam->qi[0]));
      funcOp->setAttr("keySwitchTechnique",
                      getStringAttr(schemeParam->dnum != 0 ? "HYBRID" : "BV"));
      funcOp->setAttr("digitSize", getIntegerAttr(schemeParam->digitSize));
      funcOp->setAttr("numLargeDigits", getIntegerAttr(schemeParam->dnum));
    });
  }

  void runOnOperation() override {
    ModuleOp module = getOperation();
    OpBuilder builder(module);

    multiplicationAlwaysRelinearize();
    multiplicationAlwaysModReduceBefore(includeFirst);
    alwaysModreduceWhenLevelMismatch();

    // call CSE here because there may be redundant mod reduce
    // one Value may get mod reduced multiple times in
    // multiple Uses
    OpPassManager csePipeline("builtin.module");
    csePipeline.addPass(createCSEPass());
    (void)runPipeline(csePipeline, module);
    auto maxLevel = annotateLevel();

    annotatePlaintextLevel(maxLevel);
    annotateDimension();
    annotateBound();
    annotateSchemeParams();
  }
};

}  // namespace heir
}  // namespace mlir
