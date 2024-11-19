//func.func @func(%arg0: i8, %arg1: i8) -> i8 {
//  %0 = arith.addi %arg0, %arg1 : i8
//  return %0 : i8
//}

//func.func @func(%arg0: tensor<8xi16>, %arg1: tensor<8xi16>) -> i16 {
//  %c0 = arith.constant 0 : index
//  %c0_si16 = arith.constant 0 : i16
//  %0 = affine.for %arg2 = 0 to 8 iter_args(%iter = %c0_si16) -> (i16) {
//    %1 = tensor.extract %arg0[%arg2] : tensor<8xi16>
//    %2 = tensor.extract %arg1[%arg2] : tensor<8xi16>
//    %3 = arith.muli %1, %2 : i16
//    %4 = arith.addi %iter, %3 : i16
//    affine.yield %4 : i16
//  }
//  return %0 : i16
//}

// func.func @func(
//     %arg0 : i16,
//     %arg1 : i16,
//     %arg2 : i16,
//     %arg3 : i16,
//     %arg4 : i16,
//     %arg5 : i16,
//     %arg6 : i16,
//     %arg7 : i16
//   ) -> i16 {
//     %1 = arith.muli %arg0, %arg1 :i16
//     %2 = arith.muli %1, %arg2 : i16
//     %3 = arith.muli %2, %arg3 : i16
//     %4 = arith.muli %3, %arg4 : i16
//     %5 = arith.muli %4, %arg5 : i16
//     %6 = arith.muli %5, %arg6 : i16
//     %7 = arith.muli %6, %arg7 : i16
//   return %7 : i16
// }

module {
  func.func @func(%arg0: !secret.secret<i16>, %arg1: !secret.secret<i16>, %arg2: !secret.secret<i16>, %arg3: !secret.secret<i16>, %arg4: !secret.secret<i16>, %arg5: !secret.secret<i16>, %arg6: !secret.secret<i16>, %arg7: !secret.secret<i16>) -> !secret.secret<i16> attributes {digitSize = 30 : i64, keySwitchTechnique = "BV", maxRelinSkDeg = 2 : i64, multiplicativeDepth = 6 : i64, numLargeDigits = 0 : i64, plaintextModulus = 65537 : i64, ringDim = 8192 : i64, scalingModSize = 55 : i64} {
    %0 = secret.generic ins(%arg0, %arg1, %arg2, %arg3, %arg4, %arg5, %arg6, %arg7 : !secret.secret<i16>, !secret.secret<i16>, !secret.secret<i16>, !secret.secret<i16>, !secret.secret<i16>, !secret.secret<i16>, !secret.secret<i16>, !secret.secret<i16>) attrs = {depth = 3 : i64} {
    ^bb0(%arg8: i16, %arg9: i16, %arg10: i16, %arg11: i16, %arg12: i16, %arg13: i16, %arg14: i16, %arg15: i16):
      %1 = arith.muli %arg8, %arg8 {mgmt = [["mult", "57.67 M(s 2)"]]} : i16
      %2 = arith.muli %arg8, %arg8 {mgmt = [["mult", "57.67 M(s 2)"]]} : i16
      %3 = arith.muli %1, %2 {mgmt = [["mult", "119.45 M(s 2)"]]} : i16
      %4 = arith.muli %arg8, %arg8 {mgmt = [["mult", "57.67 M(s 2)"]]} : i16
      %5 = arith.muli %arg8, %arg8 {mgmt = [["mult", "57.67 M(s 2)"]]} : i16
      %6 = arith.muli %4, %5 {mgmt = [["mult", "119.45 M(s 2)"]]} : i16
      %7 = arith.muli %3, %6 {mgmt = [["mult", "242.99 M(s 2)"]]} : i16
      secret.yield %7 : i16
    } -> !secret.secret<i16>
    return %0 : !secret.secret<i16>
  }
}
