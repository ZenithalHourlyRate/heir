load("@llvm-project//mlir:tblgen.bzl", "gentbl_cc_library", "td_library")

package(
    default_applicable_licenses = ["@heir//:license"],
    default_visibility = ["//visibility:public"],
)

cc_library(
    name = "NoiseInterfaces",
    srcs = ["NoiseInterfaces.cpp"],
    hdrs = ["NoiseInterfaces.h"],
    deps = [
        ":NoiseInterfacesIncGen",
        "@heir//lib/Analysis/NoisePropagation:Variance",
        "@llvm-project//mlir:IR",
    ],
)

td_library(
    name = "NoiseInterfacesTdFiles",
    srcs = ["NoiseInterfaces.td"],
    deps = [
        "@llvm-project//mlir:OpBaseTdFiles",
    ],
)

gentbl_cc_library(
    name = "NoiseInterfacesIncGen",
    tbl_outs = [
        (
            ["-gen-op-interface-decls"],
            "NoiseInterfaces.h.inc",
        ),
        (
            ["-gen-op-interface-defs"],
            "NoiseInterfaces.cpp.inc",
        ),
    ],
    tblgen = "@llvm-project//mlir:mlir-tblgen",
    td_file = "NoiseInterfaces.td",
    deps = [
        ":NoiseInterfacesTdFiles",
    ],
)
