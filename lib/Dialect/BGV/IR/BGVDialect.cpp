#include "lib/Dialect/BGV/IR/BGVDialect.h"

#include <optional>

#include "lib/Dialect/BGV/IR/BGVOps.h"
#include "lib/Dialect/FHEHelpers.h"
#include "lib/Dialect/LWE/IR/LWEAttributes.h"
#include "lib/Dialect/LWE/IR/LWETypes.h"
#include "llvm/include/llvm/ADT/TypeSwitch.h"            // from @llvm-project
#include "llvm/include/llvm/Support/ErrorHandling.h"     // from @llvm-project
#include "mlir/include/mlir/IR/Builders.h"               // from @llvm-project
#include "mlir/include/mlir/IR/DialectImplementation.h"  // from @llvm-project
#include "mlir/include/mlir/IR/Location.h"               // from @llvm-project
#include "mlir/include/mlir/IR/MLIRContext.h"            // from @llvm-project
#include "mlir/include/mlir/Support/LLVM.h"              // from @llvm-project

// Generated definitions
#include "lib/Dialect/BGV/IR/BGVDialect.cpp.inc"
#include "mlir/include/mlir/Support/LogicalResult.h"  // from @llvm-project
#define GET_OP_CLASSES
#include "lib/Dialect/BGV/IR/BGVOps.cpp.inc"

namespace mlir {
namespace heir {
namespace bgv {

//===----------------------------------------------------------------------===//
// BGV dialect.
//===----------------------------------------------------------------------===//

// Dialect construction: there is one instance per context and it registers its
// operations, types, and interfaces here.
void BGVDialect::initialize() {
  addOperations<
#define GET_OP_LIST
#include "lib/Dialect/BGV/IR/BGVOps.cpp.inc"
      >();
}

LogicalResult MulOp::verify() { return verifyMulOp(this); }

LogicalResult RotateOp::verify() { return verifyRotateOp(this); }

LogicalResult RelinearizeOp::verify() { return verifyRelinearizeOp(this); }

LogicalResult ModulusSwitchOp::verify() {
  return verifyModulusSwitchOrRescaleOp(this);
}

LogicalResult MulOp::inferReturnTypes(
    MLIRContext *ctx, std::optional<Location>, MulOp::Adaptor adaptor,
    SmallVectorImpl<Type> &inferredReturnTypes) {
  return inferMulOpReturnTypes(ctx, adaptor, inferredReturnTypes);
}

LogicalResult RelinearizeOp::inferReturnTypes(
    MLIRContext *ctx, std::optional<Location>, RelinearizeOp::Adaptor adaptor,
    SmallVectorImpl<Type> &inferredReturnTypes) {
  return inferRelinearizeOpReturnTypes(ctx, adaptor, inferredReturnTypes);
}

LogicalResult ExtractOp::verify() { return verifyExtractOp(this); }

void AddOp::inferResultNoise(llvm::ArrayRef<Variance> argNoises,
                             SetNoiseFn setValueNoise) {
  if (!argNoises[0].isInitialized() || !argNoises[1].isInitialized()) {
    emitOpError() << "uses SSA value with uninitialized noise variance.";
    return setValueNoise(getResult(), Variance::unbounded());
  }
  return setValueNoise(getResult(), argNoises[0] + argNoises[1]);
}
bool AddOp::hasArgumentIndependentResultNoise() { return false; }

void MulOp::inferResultNoise(llvm::ArrayRef<Variance> argNoises,
                             SetNoiseFn setValueNoise) {
  if (!argNoises[0].isInitialized() || !argNoises[1].isInitialized()) {
    emitOpError() << "uses SSA value with uninitialized noise variance.";
    return setValueNoise(getResult(), Variance::unbounded());
  }
  return setValueNoise(getResult(), argNoises[0] * argNoises[1]);
}
bool MulOp::hasArgumentIndependentResultNoise() { return false; }

void RelinearizeOp::inferResultNoise(llvm::ArrayRef<Variance> argNoises,
                                     SetNoiseFn setValueNoise) {
  if (!argNoises[0].isInitialized()) {
    emitOpError() << "uses SSA value with uninitialized noise variance.";
    return setValueNoise(getResult(), Variance::unbounded());
  }
  return setValueNoise(getResult(), argNoises[0].max(Variance::of(70)));
}
bool RelinearizeOp::hasArgumentIndependentResultNoise() { return false; }

void RotateOp::inferResultNoise(llvm::ArrayRef<Variance> argNoises,
                                SetNoiseFn setValueNoise) {
  if (!argNoises[0].isInitialized()) {
    emitOpError() << "uses SSA value with uninitialized noise variance.";
    return setValueNoise(getResult(), Variance::unbounded());
  }
  return setValueNoise(getResult(), argNoises[0].max(Variance::of(70)));
}
bool RotateOp::hasArgumentIndependentResultNoise() { return false; }

}  // namespace bgv
}  // namespace heir
}  // namespace mlir
