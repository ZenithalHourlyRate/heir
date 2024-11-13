load("@heir//lib/Transforms:transforms.bzl", "add_heir_transforms")

package(
    default_applicable_licenses = ["@heir//:license"],
    default_visibility = ["//visibility:public"],
)

cc_library(
    name = "AnnotateSecretManagement",
    srcs = [
        "AnnotateSecretManagement.cpp",
    ],
    hdrs = [
        "Passes.h",
    ],
    deps = [
        ":pass_inc_gen",
        "@heir//lib/Analysis/MulDepthAnalysis",
        "@heir//lib/Dialect/Secret/IR:Dialect",
        "@llvm-project//mlir:ArithDialect",
        "@llvm-project//mlir:FuncDialect",
        "@llvm-project//mlir:IR",
        "@llvm-project//mlir:Pass",
        "@llvm-project//mlir:TransformUtils",
        "@llvm-project//mlir:Transforms",
    ],
)

add_heir_transforms(
    header_filename = "Passes.h.inc",
    pass_name = "AnnotateSecretManagement",
    td_file = "Passes.td",
)
