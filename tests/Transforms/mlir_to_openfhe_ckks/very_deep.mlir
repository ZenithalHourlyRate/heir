// RUN: heir-opt --mlir-to-ckks="ckks-bootstrap-waterline=28 ckks-bootstrap-depth=18" --scheme-to-openfhe %s | FileCheck --check-prefix=CHECK-DEPTH_18 %s
// RUN: heir-opt --mlir-to-ckks="ckks-bootstrap-waterline=28 ckks-bootstrap-depth=22" --scheme-to-openfhe %s | FileCheck --check-prefix=CHECK-DEPTH_18 %s

// CHECK-DEPTH_18: func.func @very_deep(
// CHECK-WATERLINE_18-COUNT-2:   openfhe.bootstrap
// CHECK-WATERLINE_18-NOT:       openfhe.bootstrap

// CHECK-DEPTH_22: func.func @very_deep(
// CHECK-WATERLINE_22-COUNT-3:   openfhe.bootstrap
// CHECK-WATERLINE_22-NOT:       openfhe.bootstrap

func.func @very_deep(
    %x : f16 {secret.secret}
  ) -> f16 {
    %0 = arith.mulf %x, %x : f16
    %1 = arith.mulf %0, %0 : f16
    %2 = arith.mulf %1, %1 : f16
    %3 = arith.mulf %2, %2 : f16
    %4 = arith.mulf %3, %3 : f16
    %5 = arith.mulf %4, %4 : f16
    %6 = arith.mulf %5, %5 : f16
    %7 = arith.mulf %6, %6 : f16
    %8 = arith.mulf %7, %7 : f16
    %9 = arith.mulf %8, %8 : f16
    %10 = arith.mulf %9, %9 : f16
    %11 = arith.mulf %10, %10 : f16
    %12 = arith.mulf %11, %11 : f16
    %13 = arith.mulf %12, %12 : f16
    %14 = arith.mulf %13, %13 : f16
    %15 = arith.mulf %14, %14 : f16
    %16 = arith.mulf %15, %15 : f16
    %17 = arith.mulf %16, %16 : f16
    %18 = arith.mulf %17, %17 : f16
    %19 = arith.mulf %18, %18 : f16
    %20 = arith.mulf %19, %19 : f16
    %21 = arith.mulf %20, %20 : f16
    %22 = arith.mulf %21, %21 : f16
    %23 = arith.mulf %22, %22 : f16
    %24 = arith.mulf %23, %23 : f16
    %25 = arith.mulf %24, %24 : f16
    %26 = arith.mulf %25, %25 : f16
    %27 = arith.mulf %26, %26 : f16
    %28 = arith.mulf %27, %27 : f16
    %29 = arith.mulf %28, %28 : f16
    %30 = arith.mulf %29, %29 : f16
    %31 = arith.mulf %30, %30 : f16
    %32 = arith.mulf %31, %31 : f16
    %33 = arith.mulf %32, %32 : f16
    %34 = arith.mulf %33, %33 : f16
    %35 = arith.mulf %34, %34 : f16
    %36 = arith.mulf %35, %35 : f16
    %37 = arith.mulf %36, %36 : f16
    %38 = arith.mulf %37, %37 : f16
    %39 = arith.mulf %38, %38 : f16
  return %39 : f16
}
