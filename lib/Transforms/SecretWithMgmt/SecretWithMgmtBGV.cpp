#include "lib/Analysis/MulDepthAnalysis/MulDepthAnalysis.h"
#include "lib/Analysis/SecretnessAnalysis/SecretnessAnalysis.h"
#include "lib/Dialect/Mgmt/IR/MgmtOps.h"
#include "lib/Dialect/Secret/IR/SecretOps.h"
#include "lib/Dialect/TensorExt/IR/TensorExtOps.h"
#include "lib/Transforms/SecretWithMgmt/Passes.h"
#include "llvm/include/llvm/ADT/TypeSwitch.h"  // from @llvm-project
#include "llvm/include/llvm/Support/Debug.h"   // from @llvm-project
#include "mlir/include/mlir/Analysis/DataFlow/ConstantPropagationAnalysis.h"  // from @llvm-project
#include "mlir/include/mlir/Analysis/DataFlow/DeadCodeAnalysis.h"  // from @llvm-project
#include "mlir/include/mlir/Dialect/Arith/IR/Arith.h"   // from @llvm-project
#include "mlir/include/mlir/Dialect/Func/IR/FuncOps.h"  // from @llvm-project
#include "mlir/include/mlir/IR/Iterators.h"             // from @llvm-project
#include "mlir/include/mlir/Transforms/GreedyPatternRewriteDriver.h"  // from @llvm-project
#include "mlir/include/mlir/Transforms/Passes.h"  // from @llvm-project

#define DEBUG_TYPE "secret-with-mgmt-bgv"

namespace mlir {
namespace heir {

#define GEN_PASS_DEF_SECRETWITHMGMTBGV
#include "lib/Transforms/SecretWithMgmt/Passes.h.inc"

struct SecretWithMgmtBGV : impl::SecretWithMgmtBGVBase<SecretWithMgmtBGV> {
  using SecretWithMgmtBGVBase::SecretWithMgmtBGVBase;

  void multiplicationAlwaysRelinearizeAndModReduce() {
    OpBuilder b(&getContext());
    getOperation()->walk<WalkOrder::PreOrder>([&](secret::GenericOp genericOp) {
      genericOp.getBody()->walk<WalkOrder::PreOrder>([&](Operation *op) {
        llvm::TypeSwitch<Operation &>(*op).Case<arith::MulIOp>([&](auto mulOp) {
          b.setInsertionPointAfter(mulOp);
          Value result = mulOp.getResult();
          auto relinearized =
              b.create<mgmt::RelinearizeOp>(mulOp->getLoc(), result);
          auto modreduced =
              b.create<mgmt::ModReduceOp>(relinearized->getLoc(), relinearized);
          result.replaceAllUsesExcept(modreduced, {relinearized});
        });
      });
    });
  }

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
    // NOTE: MulDepthAnalysis works because of
    // multiplicationAlwaysRelinearizeAndModReduce
    // where modreduceop has the same mulDepthLattice.
    solver.load<MulDepthAnalysis>();
    solver.load<SecretnessAnalysis>();
    if (failed(solver.initializeAndRun(getOperation()))) {
      getOperation()->emitOpError() << "Failed to run the analysis.\n";
      signalPassFailure();
      return;
    }

    auto getMulDepth = [&](Value value) {
      auto &latticeValue =
          solver.lookupState<MulDepthLattice>(value)->getValue();
      auto mulDepth = 0;
      if (latticeValue.isInitialized()) {
        mulDepth = latticeValue.getValue();
      }
      return mulDepth;
    };

    auto getSecretness = [&](Value value) {
      auto &secretness =
          solver.lookupState<SecretnessLattice>(value)->getValue();
      return secretness;
    };

    getOperation()->walk<WalkOrder::PreOrder>([&](secret::GenericOp genericOp) {
      genericOp.getBody()->walk<WalkOrder::PreOrder>([&](Operation *op) {
        ImplicitLocOpBuilder b(op->getLoc(), op);
        llvm::TypeSwitch<Operation &>(*op).Case<arith::MulIOp, arith::AddIOp>(
            [&](auto arithOp) {
              auto mulDepthResult = getMulDepth(op->getResult(0));
              auto mulDepthOperandShould = mulDepthResult;
              if (mlir::isa<arith::MulIOp>(arithOp)) {
                mulDepthOperandShould -= 1;
              }

              auto secretnessResult = getSecretness(op->getResult(0));

              LLVM_DEBUG(llvm::dbgs()
                         << "Visiting " << arithOp << " with result mulDepth "
                         << mulDepthResult << " and secretness "
                         << secretnessResult << "\n");

              if (secretnessResult.isInitialized() &&
                  !secretnessResult.getSecretness()) {
                return;
              }

              for (auto operand : op->getOperands()) {
                auto mulDepthOperand = getMulDepth(operand);
                auto secretnessOperand = getSecretness(operand);
                LLVM_DEBUG(llvm::dbgs()
                           << "  Operand " << operand << " with mulDepth "
                           << mulDepthOperand << " and secretness "
                           << secretnessOperand << "\n");

                // skip mod reduce if operand is not secret
                if (!secretnessOperand.isInitialized() ||
                    !secretnessOperand.getSecretness()) {
                  continue;
                }

                if (mulDepthOperand < mulDepthOperandShould) {
                  Value managed = operand;
                  for (auto i = 0; i != mulDepthOperandShould - mulDepthOperand;
                       ++i) {
                    managed = b.create<mgmt::ModReduceOp>(managed);
                  }
                  op->replaceUsesOfWith(operand, managed);
                } else if (mulDepthOperand > mulDepthOperandShould) {
                  // should not happen
                  assert(false && "multiplcativeDepth analysis error");
                }
              }
            });
      });
    });
  }

  void annotateLevel() {
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
  }

  void runOnOperation() override {
    ModuleOp module = getOperation();
    OpBuilder builder(module);

    multiplicationAlwaysRelinearizeAndModReduce();
    // NOTE: not used for now
    // rotationAlwaysRelinearize();
    alwaysModreduceWhenLevelMismatch();
    annotateLevel();
  }
};

}  // namespace heir
}  // namespace mlir
