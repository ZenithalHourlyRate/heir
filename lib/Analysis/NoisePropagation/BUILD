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
        ":Symbolic",
        ":Variance",
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
        "Params.h",
        "Symbolic.h",
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
    ],
    deps = [
        "@llvm-project//llvm:Support",
        "@llvm-project//mlir:IR",
    ],
)

cc_library(
    name = "Symbolic",
    srcs = ["Symbolic.cpp"],
    hdrs = [
        "Symbolic.h",
    ],
    deps = [
        "@llvm-project//llvm:Support",
        "@llvm-project//mlir:IR",
    ],
)
