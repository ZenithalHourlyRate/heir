package(
    default_applicable_licenses = ["@heir//:license"],
    default_visibility = ["//visibility:public"],
)

cc_library(
    name = "NoisePropagationAnalysis",
    srcs = ["NoisePropagationAnalysis.cpp"],
    hdrs = ["NoisePropagationAnalysis.h"],
    deps = [
        ":Params",
        ":Variance",
        "@heir//lib/Dialect/Secret/IR:Dialect",
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
        "@heir//lib/Dialect/Secret/IR:Dialect",
        "@llvm-project//llvm:Support",
        "@llvm-project//mlir:Analysis",
        "@llvm-project//mlir:ArithDialect",
        "@llvm-project//mlir:IR",
    ],
)

cc_library(
    name = "Variance",
    srcs = ["Variance.cpp"],
    hdrs = [
        "Variance.h",
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
        "Variance.h",
    ],
    deps = [
        ":Variance",
        "@llvm-project//llvm:Support",
        "@llvm-project//mlir:IR",
    ],
)
