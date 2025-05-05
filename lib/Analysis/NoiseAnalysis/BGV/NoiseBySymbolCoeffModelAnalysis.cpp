#include <functional>

#include "lib/Analysis/DimensionAnalysis/DimensionAnalysis.h"
#include "lib/Analysis/LevelAnalysis/LevelAnalysis.h"
#include "lib/Analysis/NoiseAnalysis/BGV/NoiseBySymbolCoeffModel.h"
#include "lib/Analysis/NoiseAnalysis/NoiseAnalysis.h"
#include "lib/Analysis/Utils.h"
#include "lib/Dialect/Mgmt/IR/MgmtOps.h"
#include "lib/Dialect/Secret/IR/SecretOps.h"
#include "lib/Dialect/TensorExt/IR/TensorExtOps.h"
#include "llvm/include/llvm/ADT/TypeSwitch.h"             // from @llvm-project
#include "llvm/include/llvm/Support/Debug.h"              // from @llvm-project
#include "mlir/include/mlir/Dialect/Arith/IR/Arith.h"     // from @llvm-project
#include "mlir/include/mlir/Dialect/Tensor/IR/Tensor.h"   // from @llvm-project
#include "mlir/include/mlir/IR/Operation.h"               // from @llvm-project
#include "mlir/include/mlir/IR/Value.h"                   // from @llvm-project
#include "mlir/include/mlir/Interfaces/CallInterfaces.h"  // from @llvm-project
#include "mlir/include/mlir/Support/LLVM.h"               // from @llvm-project

#define DEBUG_TYPE "NoiseAnalysis"

namespace mlir {
namespace heir {

// explicit specialization of NoiseAnalysis for NoiseByBoundCoeffModel
template <typename NoiseModel>
void NoiseAnalysis<NoiseModel>::setToEntryState(LatticeType *lattice) {
  // At an entry point, we have no information about the noise.
  this->propagateIfChanged(lattice, lattice->join(NoiseState()));
}

// explicit specialization of NoiseAnalysis for NoiseByBoundCoeffModel
template <typename NoiseModel>
void NoiseAnalysis<NoiseModel>::visitExternalCall(
    CallOpInterface call, ArrayRef<const LatticeType *> argumentLattices,
    ArrayRef<LatticeType *> resultLattices) {
  auto callback =
      std::bind(&NoiseAnalysis<NoiseModel>::propagateIfChangedWrapper, this,
                std::placeholders::_1, std::placeholders::_2);
  ::mlir::heir::visitExternalCall<NoiseState, LatticeType>(
      call, argumentLattices, resultLattices, callback);
}

// explicit specialization of NoiseAnalysis for NoiseByBoundCoeffModel
template <typename NoiseModel>
LogicalResult NoiseAnalysis<NoiseModel>::visitOperation(
    Operation *op, ArrayRef<const LatticeType *> operands,
    ArrayRef<LatticeType *> results) {
  auto getLocalParam = [&](Value value) {
    auto level = getLevelFromMgmtAttr(value);
    auto dimension = getDimensionFromMgmtAttr(value);
    return LocalParamType(&schemeParam, level, dimension);
  };

  auto propagate = [&](Value value, NoiseState noise) {
    [[maybe_unused]] auto degreeVec = noise.getDegreeForGeneralizedNormal();
    std::string degreeVecStr = "[";
    for (auto degree : degreeVec) {
      degreeVecStr += std::to_string(degree) + " ";
    }
    degreeVecStr += "]";
    LLVM_DEBUG(llvm::dbgs()
               << "Propagating "
               << noiseModel.toLogBoundString(getLocalParam(value), noise)
               << " " << degreeVecStr << " to " << value << "\n");
    LatticeType *lattice = this->getLatticeElement(value);
    auto changeResult = lattice->join(noise);
    this->propagateIfChanged(lattice, changeResult);
  };

  // auto getOperandNoises = [&](Operation *op,
  //                             SmallVectorImpl<NoiseState> &noises) {
  //   SmallVector<OpOperand *> secretOperands;
  //   SmallVector<OpOperand *> nonSecretOperands;
  //   this->getSecretOperands(op, secretOperands);
  //   this->getNonSecretOperands(op, nonSecretOperands);

  //   for (auto *operand : secretOperands) {
  //     noises.push_back(this->getLatticeElement(operand->get())->getValue());
  //   }
  //   for (auto *operand : nonSecretOperands) {
  //     (void)operand;
  //     // at least one operand is secret
  //     auto localParam = getLocalParam(secretOperands[0]->get());
  //     noises.push_back(NoiseModel::evalConstant(localParam));
  //   }
  // };

  auto res =
      llvm::TypeSwitch<Operation &, LogicalResult>(*op)
          .Case<secret::GenericOp>([&](auto genericOp) {
            Block *body = genericOp.getBody();
            for (BlockArgument &arg : body->getArguments()) {
              auto localParam = getLocalParam(arg);
              NoiseState encrypted =
                  noiseModel.evalEncrypt(localParam, arg.getArgNumber());
              propagate(arg, encrypted);
            }
            return success();
          })
          .template Case<arith::MulIOp>([&](auto mulOp) {
            SmallVector<OpResult> secretResults;
            this->getSecretResults(mulOp, secretResults);
            if (secretResults.empty()) {
              return success();
            }

            auto localParam = getLocalParam(mulOp.getResult());
            NoiseState mult = noiseModel.evalMul(
                localParam, operands[0]->getValue(), operands[1]->getValue());
            propagate(mulOp.getResult(), mult);
            return success();
          })
          .template Case<mgmt::RelinearizeOp>([&](auto relinearizeOp) {
            propagate(relinearizeOp.getResult(), operands[0]->getValue());
            return success();
          })
          .Default([&](auto &op) { return success(); });
  return res;
}

// template instantiation
template class NoiseAnalysis<bgv::NoiseBySymbolCoeffModel>;

}  // namespace heir
}  // namespace mlir
