// func.func @func(%arg0: tensor<8xf16>, %arg1: tensor<8xf16>) -> f16 {
//   %c0 = arith.constant 0 : index
//   %c0_sf16 = arith.constant 0.0 : f16
//   %0 = affine.for %arg2 = 0 to 8 iter_args(%iter = %c0_sf16) -> (f16) {
//     %1 = tensor.extract %arg0[%arg2] : tensor<8xf16>
//     %2 = tensor.extract %arg1[%arg2] : tensor<8xf16>
//     %3 = arith.mulf %1, %2 : f16
//     %4 = arith.addf %iter, %3 : f16
//     affine.yield %4 : f16
//   }
//   return %0 : f16
// }

// func.func @func(%arg0: tensor<8xf16>, %arg1: tensor<8xf16>) -> tensor<8xf16> {
//   %0 = arith.mulf %arg0, %arg1 : tensor<8xf16>
//   return %0 : tensor<8xf16>
// }

// func.func @func(
//     %arg0 : f16
//   ) -> f16 {
//     %1 = arith.mulf %arg0, %arg0 :f16
//     %2 = arith.mulf %1, %arg0 : f16
//     %3 = arith.mulf %2, %arg0 : f16
//     %4 = arith.mulf %3, %arg0 : f16
//     %5 = arith.mulf %4, %arg0 : f16
//     %6 = arith.mulf %5, %arg0 : f16
//     %7 = arith.mulf %6, %arg0 : f16
//   return %7 : f16
// }

func.func @func(
    %arg0 : f16
  ) -> f16 {
    %1 = arith.mulf %arg0, %arg0 :f16
    %2 = arith.mulf %1, %arg0 : f16
    %3 = arith.mulf %2, %arg0 : f16
    %4 = arith.mulf %3, %arg0 : f16
    %5 = arith.mulf %4, %arg0 : f16
    %6 = arith.mulf %5, %arg0 : f16
    %7 = arith.mulf %6, %arg0 : f16
  return %7 : f16
}
