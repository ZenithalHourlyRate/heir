#ifndef LIB_DIALECT_MGMT_IR_MGMTDIALECT_TD_
#define LIB_DIALECT_MGMT_IR_MGMTDIALECT_TD_

include "mlir/IR/DialectBase.td"

def Mgmt_Dialect : Dialect {
  let name = "mgmt";
  let description = [{
    The `mgmt` dialect is a place for generic management op
    like relinearize and mod reduce
  }];

  let cppNamespace = "::mlir::heir::mgmt";
}

#endif  // LIB_DIALECT_MGMT_IR_MGMTDIALECT_TD_
