#include "lib/Transforms/ValidateNoise/ValidateNoise.h"

#include "lib/Analysis/NoisePropagation/NoisePropagationAnalysis.h"
#include "lib/Analysis/NoisePropagation/Variance.h"
#include "lib/Dialect/Mgmt/IR/MgmtOps.h"
#include "lib/Dialect/Secret/IR/SecretOps.h"
#include "llvm/include/llvm/ADT/TypeSwitch.h"  // from @llvm-project
#include "llvm/include/llvm/Support/Debug.h"   // from @llvm-project
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
    OpBuilder builder(module);

    DataFlowSolver solver;
    solver.load<dataflow::DeadCodeAnalysis>();
    solver.load<dataflow::SparseConstantPropagation>();
    solver.load<NoiseStatesAnalysis>();
    if (failed(solver.initializeAndRun(getOperation()))) {
      getOperation()->emitOpError() << "Failed to run the analysis.\n";
      signalPassFailure();
      return;
    }

    auto getVarianceStates = [&](Value value) {
      return solver.lookupState<VarianceStatesLattice>(value)->getValue();
    };

    getOperation()->walk<WalkOrder::PreOrder>([&](secret::GenericOp genericOp) {
      DenseMap<Value, std::string> valueNameMap;
      int valueName = 0;

      auto updateName = [&](Value value) {
        if (valueNameMap.find(value) == valueNameMap.end()) {
          valueNameMap[value] = std::to_string(valueName++);
        }
      };

      Block *body = genericOp.getBody();
      auto yieldOp = body->getTerminator();
      // TODO: multiple operands
      auto resultValue = yieldOp->getOperand(0);

      // currentValue, currentKey, currentParents, currentParent, previousValue
      std::vector<std::tuple<Value, VarianceKey, VarianceParents,
                             VarianceParent, Value>>
          dag;

      // init the dag
      auto vss = getVarianceStates(resultValue);
      auto key = vss.getMinimalCostKey();
      auto param = key.getParam();
      auto parents = vss.getParentsByMinCost(key);
      for (auto &parent : parents.getParents()) {
        dag.emplace_back(resultValue, key, parents, parent, resultValue);
      }

      LLVM_DEBUG(llvm::dbgs() << "Selected Param: " << param << "\n");
      // FIXME: better way!!!
      auto funcOp = genericOp->getParentOp();
      auto getIntegerAttr = [&](int64_t n) {
        return builder.getIntegerAttr(builder.getIntegerType(64), n);
      };
      auto getStringAttr = [&](std::string str) {
        return builder.getStringAttr(str);
      };
      funcOp->setAttr("ringDim", getIntegerAttr(param.n));
      funcOp->setAttr("multiplicativeDepth", getIntegerAttr(param.L));
      funcOp->setAttr("plaintextModulus", getIntegerAttr(param.t));
      funcOp->setAttr("maxRelinSkDeg", getIntegerAttr(param.maxRelinSkDeg));
      funcOp->setAttr("scalingModSize", getIntegerAttr(param.qi[0]));
      funcOp->setAttr("keySwitchTechnique",
                      getStringAttr(param.dnum != 0 ? "HYBRID" : "BV"));
      funcOp->setAttr("digitSize", getIntegerAttr(param.digitSize));
      funcOp->setAttr("numLargeDigits", getIntegerAttr(param.dnum));

      // tarverse the parent dag

      auto getParentValue = [&](Value current, VarianceParent parent) {
        auto type = parent.getType();
        if (type == VarianceParentType::Self) {
          return current;
        } else if (type == VarianceParentType::Operand0) {
          return current.getDefiningOp()->getOperand(0);
        } else if (type == VarianceParentType::Operand1) {
          return current.getDefiningOp()->getOperand(1);
        }
        return current;
      };

      // to value, from value, middle value key, middle value parents (edge
      // reason)
      std::vector<std::tuple<Value, Value, VarianceKey, VarianceParents>> route;
      auto updateRoute = [&](Value to, Value from, VarianceKey middleKey,
                             VarianceParents middleParents) {
        route.emplace_back(to, from, middleKey, middleParents);
      };

      size_t index = 0;
      while (index != dag.size()) {
        auto [currentValue, currentKey, currentParents, currentParent,
              previousValue] = dag[index];

        auto parentValue = getParentValue(currentValue, currentParent);
        auto *parentKey = currentParent.getParentKey();

        Value previous = previousValue;
        if (parentValue != currentValue) {
          previous = currentValue;
        }

        updateRoute(previous, parentValue, currentKey, currentParents);

        auto parentParents =
            getVarianceStates(parentValue)
                .getParentsBySuccessorParent(*parentKey, currentParent);
        for (auto &parentParent : parentParents.getParents()) {
          dag.emplace_back(parentValue, *parentKey, parentParents, parentParent,
                           previous);
        }

        index++;
      }

      auto selected = [&](Value to0, Value from0) {
        std::vector<std::tuple<VarianceKey, VarianceParents>> selected;
        for (auto &[to, from, key, parents] : route) {
          if (to0 == to && from0 == from) {
            selected.emplace_back(key, parents);
          }
        }
        std::reverse(selected.begin(), selected.end());
        return selected;
      };

      auto selected_bounds = [&](Value to, Value from) {
        auto vssTo = getVarianceStates(to);
        auto vssFrom = getVarianceStates(from);
        auto sel = selected(to, from);
        std::vector<std::tuple<std::string, std::string, std::string>> bounds;
        for (auto &[key, parents] : sel) {
          bool isMgmt =
              parents.getReason() == "relin" || parents.getReason() == "modd";
          auto &vss = isMgmt ? vssFrom : vssTo;
          bounds.emplace_back(
              parents.getReason(),
              key.toBound(
                  vss.getExpressionVarianceByCurrentParents(key, parents)),
              vss.getExpressionByCurrentParents(key, parents).toString());
        }
        return bounds;
      };

      // auto dumpDOT = [&](Value result) {
      //   auto vss = getVarianceStates(result);

      //   std::vector<Value> values;
      //   values.push_back(result);
      //   auto definingOp = result.getDefiningOp();
      //   if (definingOp) {
      //     for (auto operand : definingOp->getOperands()) {
      //       values.push_back(operand);
      //     }
      //   }

      //   updateName(result);

      //   LLVM_DEBUG(llvm::dbgs()
      //              << vss.toDOTNode(values, valueNameMap, selected(result))
      //              << vss.toDOTEdge(values, valueNameMap, selected(result)));
      //   return WalkResult::advance();
      // };

      auto dumpBound = [&](Value to, Value from) {
        auto bounds = selected_bounds(to, from);
        for (auto &[reason, bound, symbol] : bounds) {
          LLVM_DEBUG(llvm::dbgs()
                     << to << " " << from << ": " << reason << " bound "
                     << bound << " symbol " << symbol << "\n");
        }
      };

      auto manageValue = [&](ImplicitLocOpBuilder &b, Value to, Value from) {
        auto vssTo = getVarianceStates(to);
        auto vssFrom = getVarianceStates(from);

        auto sel = selected(to, from);
        Value operandManaged = from;
        for (auto &[key, parents] : sel) {
          if (parents.getReason() == "modd") {
            operandManaged = b.create<mgmt::ModReduceOp>(operandManaged);
          } else if (parents.getReason() == "relin") {
            operandManaged = b.create<mgmt::RelinearizeOp>(operandManaged);
          }

          bool isMgmt =
              parents.getReason() == "relin" || parents.getReason() == "modd";
          auto &vss = isMgmt ? vssFrom : vssTo;
#ifndef IGNORE_SYMBOL
          auto bound = key.toBound(
              vss.getExpressionVarianceByCurrentParents(key, parents));
          auto expr =
              vss.getExpressionByCurrentParents(key, parents).toString();
#else
          auto bound =
              key.toBound(vss.getVarianceByCurrentParents(key, parents));
#endif

          auto boundAttr = builder.getStringAttr(bound);
          if (isMgmt) {
            operandManaged.getDefiningOp()->setAttr("bound", boundAttr);
          } else {
            to.getDefiningOp()->setAttr("bound", boundAttr);
          }
        }
        return operandManaged;
      };

      body->walk([&](Operation *op) {
        ImplicitLocOpBuilder b(op->getLoc(), op);
        llvm::TypeSwitch<Operation &>(*op)
            .Case<secret::YieldOp>([&](auto yieldOp) {
              // TODO: multiple operands
              auto operand = yieldOp.getOperand(0);
              // dumpBound(operand, operand);
              yieldOp->replaceUsesOfWith(operand,
                                         manageValue(b, operand, operand));
            })
            .Default([&](auto &op) {
              for (OpResult result : op.getResults()) {
                for (Value operand : op.getOperands()) {
                  // dumpBound(result, operand);
                  op.replaceUsesOfWith(operand,
                                       manageValue(b, result, operand));
                }
              }
            });
        return WalkResult::advance();
      });
    });
  }
};

}  // namespace heir
}  // namespace mlir
