#include "lib/Transforms/ValidateNoise/ValidateNoise.h"

#include "lib/Analysis/NoisePropagation/NoisePropagationAnalysis.h"
#include "lib/Analysis/NoisePropagation/Variance.h"
#include "lib/Dialect/Secret/IR/SecretOps.h"
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

      std::vector<std::tuple<Value, VarianceKey, VarianceParent>> tree;
      std::vector<std::tuple<Value, VarianceKey, VarianceParents>> all_selected;

      // init the tree
      auto vss = getVarianceStates(resultValue);
      auto key = vss.getMinimalCostKey();
      auto parents = vss.getParentsByMinCost(key);
      for (auto &parent : parents.getParents()) {
        tree.emplace_back(resultValue, key, parent);
      }
      all_selected.emplace_back(resultValue, key, parents);

      // tarverse the parent tree

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

      size_t index = 0;
      while (index != tree.size()) {
        auto [currentValue, currentKey, currentParent] = tree[index];

        auto parentValue = getParentValue(currentValue, currentParent);
        auto *parentKey = currentParent.getParentKey();

        auto parentParents =
            getVarianceStates(parentValue)
                .getParentsBySuccessorParent(*parentKey, currentParent);
        for (auto &parentParent : parentParents.getParents()) {
          tree.emplace_back(parentValue, *parentKey, parentParent);
        }
        all_selected.emplace_back(parentValue, *parentKey, parentParents);

        index++;
      }

      // for (auto &[value, key, parent] : tree) {
      //   LLVM_DEBUG(llvm::dbgs()
      //              << "value " << value << " key " << key << " parnetKey "
      //              << *parent.getParentKey() << "\n");
      // }

      auto selected = [&](Value result) {
        std::vector<std::tuple<VarianceKey, VarianceParents>> selected;
        for (auto &[value, key, parents] : all_selected) {
          if (value == result) {
            selected.emplace_back(key, parents);
          }
        }
        return selected;
      };

      auto selected_ops = [&](Value result) {
        auto sel = selected(result);
        std::vector<std::string> ops;
        for (auto &[_, parents] : sel) {
          ops.push_back(parents.getReason());
        }
        std::reverse(ops.begin(), ops.end());
        return ops;
      };

      auto dumpDOT = [&](Value result) {
        auto vss = getVarianceStates(result);

        std::vector<std::tuple<VarianceKey, VarianceParents>> selected;

        std::vector<Value> values;
        values.push_back(result);
        auto definingOp = result.getDefiningOp();
        if (definingOp) {
          for (auto operand : definingOp->getOperands()) {
            values.push_back(operand);
          }
        }
#if 0
        updateName(result);
        LLVM_DEBUG(llvm::dbgs()
                   << vss.toDOTNode(values, valueNameMap)
                   << vss.toDOTEdge(values, valueNameMap, selected(result)));
#endif
        return WalkResult::advance();
      };

      auto dumpOps = [&](Value result) {
        auto ops = selected_ops(result);
        LLVM_DEBUG(llvm::dbgs() << result << ": ");
        for (auto reason : ops) {
          LLVM_DEBUG(llvm::dbgs() << reason << " ");
        }
        LLVM_DEBUG(llvm::dbgs() << "\n");
      };

      auto concatMgmtOps = [&](Value result) {
        auto ops = selected_ops(result);
        std::vector<Attribute> mgmt_arr;
        for (size_t i = 1; i != ops.size(); ++i) {
          mgmt_arr.push_back(builder.getStringAttr(ops[i]));
        }
        return mgmt_arr;
      };

      for (size_t i = 0; i != body->getNumArguments(); ++i) {
        // dumpDOT(arg);
        auto arg = body->getArgument(i);
        // TODO: set it elsewhere
        // genericOp->setAttr("mgmt_arg" + std::to_string(i),
        //                   builder.getStringAttr(concatMgmtOps(arg)));
      }

      body->walk([&](Operation *op) {
        for (OpResult result : op->getResults()) {
          // return dumpDOT(result);
          dumpOps(result);
          op->setAttr("mgmt", builder.getArrayAttr(
                                  ArrayRef<Attribute>(concatMgmtOps(result))));
#if 0
          auto &vss = opRange->getValue();
          auto params = vss.reachable();
          if (params.size() != 0) {
            LLVM_DEBUG(llvm::dbgs()
                       << "Reachable params for " << valueNameMap.at(result)
                       << " " << result << "\n");
            for (auto &p : params) {
              auto pa = p.first;
              auto costs = p.second;
              auto cmin = std::min_element(costs.begin(), costs.end());
              LLVM_DEBUG(llvm::dbgs()
                         << pa << " min cost " << int(*cmin) << " cost [");
              int count = 0;
              for (auto &c : costs) {
                if (count++ > 4) {
                  LLVM_DEBUG(llvm::dbgs() << "...");
                  break;
                }
                LLVM_DEBUG(llvm::dbgs() << int(c) << " ");
              }
              LLVM_DEBUG(llvm::dbgs() << "]\n");
            }
          } else {
            LLVM_DEBUG(llvm::dbgs() << "No reachable params for "
                                    << valueNameMap.at(result) << "\n");
          }
#endif
        }
        return WalkResult::advance();
      });
    });
  }
};

}  // namespace heir
}  // namespace mlir
