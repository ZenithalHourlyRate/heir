#include "lib/Analysis/NoisePropagation/ParamAnalysis.h"

#include "lib/Dialect/Secret/IR/SecretOps.h"
#include "llvm/include/llvm/ADT/TypeSwitch.h"          // from @llvm-project
#include "llvm/include/llvm/Support/Debug.h"           // from @llvm-project
#include "mlir/include/mlir/Dialect/Arith/IR/Arith.h"  // from @llvm-project
#include "mlir/include/mlir/IR/Operation.h"            // from @llvm-project
#include "mlir/include/mlir/IR/Value.h"                // from @llvm-project
#include "src/pke/include/openfhe.h"                   // from @openfhe

#define DEBUG_TYPE "ParamAnalysis"

namespace mlir {
namespace heir {

// copied from OpenFHE
int computeDnum(int depth) {
  if (depth > 3) {
    return 3;
  }
  if (depth > 0) {
    return 2;
  }
  return 1;
}

int64_t getPlaintextModulus() {
  return 65537;       // 2^16 + 1
  return 786433;      // 2^19 + 2^18 + 1
  return 536903681;   // 2^29 + 2^15 + 1
  return 1073479681;  // 2^30 - 2^18 + 1
  return 4295294977;  // 2^32 + 2^18 + 2^16 + 1
}

#define HYBRID

const SchemeParam *getDefaultSchemeParam(int depth) {
  auto t = getPlaintextModulus();
  auto qiSize = int(ceil(log(t) / log(2))) + 28;  // conservative estimation...
  if (qiSize > 60) {
    qiSize = 60;
  }
#ifdef HYBRID
  auto digitSize = 0;
  auto dnum = computeDnum(depth);
#else
  auto digitSize = 0;
  auto dnum = 0;
#endif
  const auto *defaultSchemeParam =
      SchemeParamsFactory::getSchemeParam(depth, digitSize, dnum, t, qiSize, 2);
  LLVM_DEBUG(llvm::dbgs() << "Default scheme param " << *defaultSchemeParam
                          << "\n");
  return defaultSchemeParam;
}

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
              auto schemeParam = getDefaultSchemeParam(level);
              auto localParam =
                  LocalParamFactory::getLocalParam(schemeParam, 2, level);

              LLVM_DEBUG(llvm::dbgs() << "BlockArg " << i << " Local param "
                                      << *localParam << "\n");

              auto blockArg = body->getArgument(i);
              propagate(blockArg, LocalParamState(*localParam));
            }
            return success();
          })
          .Case<arith::ConstantOp>([&](auto constantOp) {
            auto levelAttr =
                dyn_cast<IntegerAttr>(constantOp->getAttr("level_scheme"));
            if (!levelAttr) {
              return failure();
            }
            auto level = levelAttr.getValue().getLimitedValue();
            auto schemeParam = getDefaultSchemeParam(level);
            auto localParam =
                LocalParamFactory::getLocalParam(schemeParam, 2, level);

            LLVM_DEBUG(llvm::dbgs() << "Constant " << constantOp.getResult()
                                    << " Local param " << *localParam << "\n");
            propagate(constantOp.getResult(), LocalParamState(*localParam));
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
            auto dimensionAttr = dyn_cast<IntegerAttr>(op.getAttr("dimension"));
            if (!dimensionAttr) {
              return success();
            }
            auto level = levelAttr.getValue().getLimitedValue();
            auto dimension = dimensionAttr.getValue().getLimitedValue();
            // inherit scheme param from operand[0]
            auto operandLattice = operands[0]->getValue();
            const auto *operandSchemeParam =
                operandLattice.getLocalParam().getSchemeParam();
            auto localParam = LocalParamFactory::getLocalParam(
                operandSchemeParam, dimension, level);

            LLVM_DEBUG(llvm::dbgs() << "Value " << op.getResult(0)
                                    << " Local param " << *localParam << "\n");
            propagate(op.getResult(0), LocalParamState(*localParam));
            return success();
          });
  return res;
}

}  // namespace heir
}  // namespace mlir
