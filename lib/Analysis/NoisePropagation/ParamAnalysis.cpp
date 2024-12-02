#include "lib/Analysis/NoisePropagation/ParamAnalysis.h"

#include "lib/Dialect/Secret/IR/SecretOps.h"
#include "llvm/include/llvm/ADT/TypeSwitch.h"          // from @llvm-project
#include "llvm/include/llvm/Support/Debug.h"           // from @llvm-project
#include "mlir/include/mlir/Dialect/Arith/IR/Arith.h"  // from @llvm-project
#include "mlir/include/mlir/IR/Operation.h"            // from @llvm-project
#include "mlir/include/mlir/IR/Value.h"                // from @llvm-project

#define DEBUG_TYPE "ParamAnalysis"

namespace mlir {
namespace heir {

LogicalResult ParamAnalysis::visitOperation(
    Operation *op, ArrayRef<const ParamLattice *> operands,
    ArrayRef<ParamLattice *> results) {
  auto propagate = [&](Value value, const LocalParamState &state) {
    auto *lattice = getLatticeElement(value);
    ChangeResult changed = lattice->join(state);
    propagateIfChanged(lattice, changed);
  };

  auto res =
      llvm::TypeSwitch<Operation &, LogicalResult>(*op)
          .Case<secret::GenericOp>([&](auto genericOp) {
            Block *body = genericOp.getBody();
            LLVM_DEBUG(llvm::dbgs()
                       << "Visiting secret genericOp with block arg "
                       << body->getArguments().size() << "\n");
            for (auto i = 0; i != body->getNumArguments(); ++i) {
              auto levelAttr =
                  dyn_cast<IntegerAttr>(genericOp.getArgAttr(i, "level"));
              if (!levelAttr) {
                return failure();
              }
              auto level = levelAttr.getValue().getLimitedValue();
              auto schemeParam = SchemeParamsFactory::getSchemeParam(
                  level, 30, 0, 65537, 55, 2);
              auto localParam = LocalParamFactory::getLocalParam(schemeParam, 2,
                                                                 level, false);

              LLVM_DEBUG(llvm::dbgs() << "BlockArg " << i << " Local param "
                                      << *localParam << "\n");

              auto blockArg = body->getArgument(i);
              propagate(blockArg, LocalParamState(*localParam));
            }
            return success();
          })
          .Default([&](auto &op) {
            auto attr = op.getAttr("level");
            if (!attr) {
              return success();
            }
            auto levelAttr = dyn_cast<IntegerAttr>(attr);
            if (!levelAttr) {
              return success();
            }
            auto level = levelAttr.getValue().getLimitedValue();
            // inherit scheme param from operand[0]
            auto operandLattice = operands[0]->getValue();
            auto operandSchemeParam =
                operandLattice.getLocalParam().getSchemeParam();
            auto localParam = LocalParamFactory::getLocalParam(
                operandSchemeParam, 2, level, false);

            LLVM_DEBUG(llvm::dbgs() << "Value " << op.getResult(0)
                                    << " Local param " << *localParam << "\n");
            propagate(op.getResult(0), LocalParamState(*localParam));
            return success();
          });
  return res;
}

}  // namespace heir
}  // namespace mlir
