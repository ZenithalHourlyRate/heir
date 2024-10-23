load("@llvm-project//mlir:tblgen.bzl", "gentbl_cc_library")

package(
    default_applicable_licenses = ["@heir//:license"],
    default_visibility = ["//visibility:public"],
)

cc_library(
    name = "ValidateNoise",
    srcs = ["ValidateNoise.cpp"],
    hdrs = ["ValidateNoise.h"],
    deps = [
        ":pass_inc_gen",
        "@heir//lib/Analysis/NoisePropagation:NoisePropagationAnalysis",
        "@heir//lib/Analysis/NoisePropagation:Variance",
        "@heir//lib/Dialect/LWE/IR:Dialect",
        "@llvm-project//llvm:Support",
        "@llvm-project//mlir:Analysis",
        "@llvm-project//mlir:Pass",
        "@llvm-project//mlir:Transforms",
    ],
)

gentbl_cc_library(
    name = "pass_inc_gen",
    tbl_outs = [
        (
            [
                "-gen-pass-decls",
                "-name=ValidateNoise",
            ],
            "ValidateNoise.h.inc",
        ),
        (
            ["-gen-pass-doc"],
            "ValidateNoise.md",
        ),
    ],
    tblgen = "@llvm-project//mlir:mlir-tblgen",
    td_file = "ValidateNoise.td",
    deps = [
        "@llvm-project//mlir:OpBaseTdFiles",
        "@llvm-project//mlir:PassBaseTdFiles",
    ],
)
