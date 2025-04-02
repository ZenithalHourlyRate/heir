package(
    default_applicable_licenses = ["@heir//:license"],
    default_visibility = ["//visibility:public"],
)

cc_library(
    name = "SymbolAnalysis",
    srcs = ["SymbolAnalysis.cpp"],
    hdrs = ["SymbolAnalysis.h"],
    deps = [
        "@heir//lib/Analysis:Utils",
        "@heir//lib/Analysis/SecretnessAnalysis",
        "@heir//lib/Dialect/Secret/IR:Dialect",
        "@llvm-project//llvm:Support",
        "@llvm-project//mlir:Analysis",
        "@llvm-project//mlir:CallOpInterfaces",
        "@llvm-project//mlir:IR",
        "@llvm-project//mlir:Support",
    ],
)
