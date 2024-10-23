#ifndef INCLUDE_TRANSFORMS_VALIDATENOISE_VALIDATENOISE_H_
#define INCLUDE_TRANSFORMS_VALIDATENOISE_VALIDATENOISE_H_

#include "mlir/include/mlir/Pass/Pass.h"  // from @llvm-project

namespace mlir {
namespace heir {

#define GEN_PASS_DECL
#include "lib/Transforms/ValidateNoise/ValidateNoise.h.inc"

#define GEN_PASS_REGISTRATION
#include "lib/Transforms/ValidateNoise/ValidateNoise.h.inc"

}  // namespace heir
}  // namespace mlir

#endif  // INCLUDE_TRANSFORMS_VALIDATENOISE_VALIDATENOISE_H_
