package(
    default_applicable_licenses = ["@heir//:license"],
    default_visibility = ["//visibility:public"],
)

cc_library(
    name = "NoisePropagationAnalysis",
    srcs = ["NoisePropagationAnalysis.cpp"],
    hdrs = ["NoisePropagationAnalysis.h"],
    deps = [
        ":NoiseKPZ21",
        ":ParamAnalysis",
        ":Params",
        ":Variance",
        "@heir//lib/Dialect/Mgmt/IR:Dialect",
        "@heir//lib/Dialect/Secret/IR:Dialect",
        "@heir//lib/Dialect/TensorExt/IR:Dialect",
        "@llvm-project//llvm:Support",
        "@llvm-project//mlir:Analysis",
        "@llvm-project//mlir:ArithDialect",
        "@llvm-project//mlir:IR",
    ],
)

cc_library(
    name = "ParamAnalysis",
    srcs = ["ParamAnalysis.cpp"],
    hdrs = ["ParamAnalysis.h"],
    deps = [
        ":Params",
        "@heir//lib/Dialect/Mgmt/IR:Dialect",
        "@heir//lib/Dialect/Secret/IR:Dialect",
        "@llvm-project//llvm:Support",
        "@llvm-project//mlir:Analysis",
        "@llvm-project//mlir:ArithDialect",
        "@llvm-project//mlir:IR",
        "@openfhe//:core",
        "@openfhe//:pke",
    ],
)

cc_library(
    name = "Variance",
    srcs = ["Variance.cpp"],
    hdrs = [
        "Params.h",
        "Variance.h",
    ],
    deps = [
        "@llvm-project//llvm:Support",
        "@llvm-project//mlir:IR",
    ],
)

cc_library(
    name = "NoiseKPZ21",
    srcs = ["NoiseKPZ21.cpp"],
    hdrs = [
        "NoiseKPZ21.h",
        "Params.h",
    ],
    deps = [
        "@llvm-project//llvm:Support",
        "@llvm-project//mlir:IR",
    ],
)

cc_library(
    name = "Params",
    srcs = ["Params.cpp"],
    hdrs = [
        "Params.h",
    ],
    deps = [
        "@llvm-project//llvm:Support",
        "@llvm-project//mlir:IR",
        "@openfhe//:core",
        "@openfhe//:pke",
    ],
)
