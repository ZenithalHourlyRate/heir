// RUN: heir-opt %s --mod-arith-to-arith --heir-polynomial-to-llvm \
// RUN:   | mlir-cpu-runner -e test_lower_rep -entry-point-result=void \
// RUN:      --shared-libs="%mlir_lib_dir/libmlir_c_runner_utils%shlibext,%mlir_runner_utils" > %t
// RUN: FileCheck %s --check-prefix=CHECK_TEST_REP < %t

func.func private @printMemrefI32(memref<*xi32>) attributes { llvm.emit_c_interface }

func.func @test_lower_rep() {
  // rep intends the input to be signed
  // 67108862 = 2 ** 26 - 2, equivalent to -2 as input
  %x = arith.constant dense<[29498763, 42, 67108862, 7681, -1, 7680]> : tensor<6xi26>
  // apply ConvertRepSimple
  %1 = mod_arith.rep %x { modulus = 7681 } : tensor<6xi26>
  // CHECK_TEST_REP: [3723, 42, 7679, 0, 7680, 7680]

  %2 = arith.extui %1 : tensor<6xi26> to tensor<6xi32>
  %3 = bufferization.to_memref %2 : memref<6xi32>
  %U = memref.cast %3 : memref<6xi32> to memref<*xi32>
  func.call @printMemrefI32(%U) : (memref<*xi32>) -> ()


  // 67108862 = 2 ** 26 - 2, equivalent to -2 as input,
  //   result should be mod - 2, namely 67108861
  // NOTE: input(67108863) rep modulus(67108863) = 67108862, weird thing!
  %y = arith.constant dense<[29498763, 42, 67108862, 67108863, -1, 7680]> : tensor<6xi26>
  // mod = 67108863 = 2 ** 26 - 1
  // apply ConvertRep w/ modulus width = type width
  %4 = mod_arith.rep %y { modulus = 67108863 } : tensor<6xi26>
  // CHECK_TEST_REP: [29498763, 42, 67108861, 67108862, 67108862, 7680]

  %5 = arith.extui %4 : tensor<6xi26> to tensor<6xi32>
  %6 = bufferization.to_memref %5 : memref<6xi32>
  %V = memref.cast %6 : memref<6xi32> to memref<*xi32>
  func.call @printMemrefI32(%V) : (memref<*xi32>) -> ()

  // 67108864 = 2 ** 26
  %7 = mod_arith.rep %y { modulus = 67108864 } : tensor<6xi26>
  // CHECK_TEST_REP: [29498763, 42, 67108862, 67108863, 67108863, 7680]

  %8 = arith.extui %7 : tensor<6xi26> to tensor<6xi32>
  %9 = bufferization.to_memref %8 : memref<6xi32>
  %W = memref.cast %9 : memref<6xi32> to memref<*xi32>
  func.call @printMemrefI32(%W) : (memref<*xi32>) -> ()

  // 33554431 = 2 ** 25 - 1
  %10 = mod_arith.rep %y { modulus = 33554431 } : tensor<6xi26>
  // CHECK_TEST_REP: [29498763, 42, 33554429, 33554430, 33554430, 7680]

  %11 = arith.extui %10 : tensor<6xi26> to tensor<6xi32>
  %12 = bufferization.to_memref %11 : memref<6xi32>
  %Y = memref.cast %12 : memref<6xi32> to memref<*xi32>
  func.call @printMemrefI32(%Y) : (memref<*xi32>) -> ()
  return
}
