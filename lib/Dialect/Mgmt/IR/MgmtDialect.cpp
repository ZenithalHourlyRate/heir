#include "lib/Dialect/Mgmt/IR/MgmtDialect.h"

#include "mlir/include/mlir/IR/DialectImplementation.h"  // from @llvm-project

// NOLINTNEXTLINE(misc-include-cleaner): Required to define MgmtOps

#include "lib/Dialect/Mgmt/IR/MgmtOps.h"

// Generated definitions
#include "lib/Dialect/Mgmt/IR/MgmtDialect.cpp.inc"

#define GET_OP_CLASSES
#include "lib/Dialect/Mgmt/IR/MgmtOps.cpp.inc"

namespace mlir {
namespace heir {
namespace mgmt {

void MgmtDialect::initialize() {
  addOperations<
#define GET_OP_LIST
#include "lib/Dialect/Mgmt/IR/MgmtOps.cpp.inc"
      >();
}

}  // namespace mgmt
}  // namespace heir
}  // namespace mlir
