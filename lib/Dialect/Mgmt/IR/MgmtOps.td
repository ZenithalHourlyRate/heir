#ifndef LIB_DIALECT_MGMT_IR_MGMTOPS_TD_
#define LIB_DIALECT_MGMT_IR_MGMTOPS_TD_

include "lib/Dialect/Mgmt/IR/MgmtDialect.td"

include "mlir/IR/BuiltinAttributes.td"
include "mlir/IR/CommonTypeConstraints.td"
include "mlir/IR/OpBase.td"
include "mlir/Interfaces/InferTypeOpInterface.td"
include "mlir/Interfaces/SideEffectInterfaces.td"

class Mgmt_Op<string mnemonic, list<Trait> traits = [Pure, ElementwiseMappable, SameOperandsAndResultType]> :
        Op<Mgmt_Dialect, mnemonic, traits> {
  let cppNamespace = "::mlir::heir::mgmt";
}

def ModArith_ModReduceOp : Mgmt_Op<"modreduce"> {
  let summary = "";

  let description = [{
  }];

  let arguments = (ins
    AnyType:$input
  );
  let results = (outs AnyType:$output);
  let assemblyFormat = "operands attr-dict `:` type($output)";
}

def ModArith_RelinearizeOp : Mgmt_Op<"relinearize"> {
  let summary = "";

  let description = [{
  }];

  let arguments = (ins
    AnyType:$input
  );
  let results = (outs AnyType:$output);
  let assemblyFormat = "operands attr-dict `:` type($output)";
}

#endif  // LIB_DIALECT_MGMT_IR_MGMTOPS_TD_
