#include "lib/Analysis/NoisePropagation/NoisePropagationAnalysis.h"

#include "lib/Analysis/NoisePropagation/ParamAnalysis.h"
#include "lib/Dialect/Mgmt/IR/MgmtOps.h"
#include "lib/Dialect/Secret/IR/SecretOps.h"
#include "lib/Dialect/TensorExt/IR/TensorExtOps.h"
#include "llvm/include/llvm/ADT/TypeSwitch.h"          // from @llvm-project
#include "llvm/include/llvm/Support/Debug.h"           // from @llvm-project
#include "mlir/include/mlir/Dialect/Arith/IR/Arith.h"  // from @llvm-project
#include "mlir/include/mlir/IR/Operation.h"            // from @llvm-project
#include "mlir/include/mlir/IR/Value.h"                // from @llvm-project

#define DEBUG_TYPE "NoisePropagationAnalysis"

namespace mlir {
namespace heir {

LogicalResult NoiseAnalysis::visitOperation(
    Operation *op, ArrayRef<const NoiseLattice *> operands,
    ArrayRef<NoiseLattice *> results) {
  auto getLocalParam = [&](Value value) -> std::optional<LocalParam> {
    const auto *paramLattice =
        getOrCreateFor<ParamLattice>(getProgramPointBefore(op), value);
    if (paramLattice->getValue().isInitialized()) {
      return paramLattice->getValue().getLocalParam();
    }
    return std::nullopt;
  };

  auto propagate = [&](Value value, NoiseType noise) {
    auto localParam = getLocalParam(value).value();

    LLVM_DEBUG(llvm::dbgs() << "Propagating " << noise.toBound(localParam)
                            << " to " << value << "\n");
    NoiseLattice *lattice = getLatticeElement(value);
    auto changeResult = lattice->join(noise);
    propagateIfChanged(lattice, changeResult);
  };

  auto res =
      llvm::TypeSwitch<Operation &, LogicalResult>(*op)
          .Case<secret::GenericOp>([&](auto genericOp) {
            Block *body = genericOp.getBody();
            for (Value &arg : body->getArguments()) {
              auto localParamOpt = getLocalParam(arg);
              if (!localParamOpt.has_value()) {
                return success();
              }

              auto localParam = *localParamOpt;

              NoiseType encrypted = NoiseType::evalEncryptPk(localParam);
              propagate(arg, encrypted);
            }
            return success();
          })
          .Case<arith::ConstantOp>([&](auto constantOp) {
            auto localParamOpt = getLocalParam(constantOp.getResult());
            if (!localParamOpt.has_value()) {
              return success();
            }

            auto localParam = *localParamOpt;
            NoiseType constant = NoiseType::evalConstant(localParam);
            propagate(constantOp.getResult(), constant);
            return success();
          })
          .Case<arith::MulIOp>([&](auto mulOp) {
            auto localParamOpt = getLocalParam(mulOp.getResult());
            if (!localParamOpt.has_value()) {
              return success();
            }

            auto localParam = *localParamOpt;
            NoiseType mult = NoiseType::evalMultNoRelin(
                localParam, operands[0]->getValue(), operands[1]->getValue());
            propagate(mulOp.getResult(), mult);
            return success();
          })
          .Case<arith::AddIOp>([&](auto addOp) {
            NoiseType add = NoiseType::evalAdd(operands[0]->getValue(),
                                               operands[1]->getValue());
            propagate(addOp.getResult(), add);
            return success();
          })
          .Case<tensor_ext::RotateOp>([&](auto rotateOp) {
            auto localParamOpt = getLocalParam(rotateOp.getOperand(0));
            if (!localParamOpt.has_value()) {
              return success();
            }

            auto localParam = *localParamOpt;
            NoiseType rotate =
                NoiseType::evalRotate(localParam, operands[0]->getValue());
            propagate(rotateOp.getResult(), rotate);
            return success();
          })
          .Case<mgmt::ModReduceOp>([&](auto modReduceOp) {
            auto localParamOpt = getLocalParam(modReduceOp.getInput());
            if (!localParamOpt.has_value()) {
              return success();
            }

            auto localParam = *localParamOpt;
            NoiseType modReduce =
                NoiseType::evalModReduce(localParam, operands[0]->getValue());
            propagate(modReduceOp.getResult(), modReduce);
            return success();
          })
          .Case<mgmt::RelinearizeOp>([&](auto relinearizeOp) {
            auto localParamOpt = getLocalParam(relinearizeOp.getInput());
            if (!localParamOpt.has_value()) {
              return success();
            }

            auto localParam = *localParamOpt;
            NoiseType relinearize =
                NoiseType::evalRelinearize(localParam, operands[0]->getValue());
            propagate(relinearizeOp.getResult(), relinearize);
            return success();
          })
          .Default([&](auto &op) { return success(); });
  return res;
}

}  // namespace heir
}  // namespace mlir
