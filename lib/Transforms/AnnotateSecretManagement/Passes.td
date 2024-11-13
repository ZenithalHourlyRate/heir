#ifndef LIB_TRANSFORMS_ANNOTATESECRETMANEGEMENT_PASSES_TD_
#define LIB_TRANSFORMS_ANNOTATESECRETMANEGEMENT_PASSES_TD_

include "mlir/Pass/PassBase.td"

def AnnotateSecretManagement : Pass<"annotate-secret-management", "ModuleOp"> {
  let summary = "Adds management attributes to arith operations";

  let description = [{
  }];

  let dependentDialects = [
    "mlir::heir::secret::SecretDialect",
    "mlir::func::FuncDialect"
  ];
}

#endif  // LIB_TRANSFORMS_ANNOTATESECRETMANEGEMENT_PASSES_TD_
