#ifndef LIB_TRANSFORMS_SECRETWITHMGMT_PASSES_TD_
#define LIB_TRANSFORMS_SECRETWITHMGMT_PASSES_TD_

include "mlir/Pass/PassBase.td"

def SecretWithMgmtBGV : Pass<"secret-with-mgmt-bgv", "ModuleOp"> {
  let summary = "Place BGV Management operations";

  let description = [{
    This implements the very basic placement of BGV management operations,
    including relinearize and mod reduce.

    Also, it annotates the level information for each operation.
  }];

  let dependentDialects = [
    "mlir::heir::secret::SecretDialect",
    "mlir::heir::mgmt::MgmtDialect",
    "mlir::func::FuncDialect"
  ];
}

#endif  // LIB_TRANSFORMS_SECRETWITHMGMT_PASSES_TD_
