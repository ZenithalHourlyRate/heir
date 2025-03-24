# See README.md for setup required to run these tests

load("@heir//tests/Examples/openfhe:test.bzl", "openfhe_end_to_end_test")

package(default_applicable_licenses = ["@heir//:license"])

openfhe_end_to_end_test(
    name = "mult_dep_8_debug_test",
    generated_lib_header = "mult_dep_8_debug_lib.h",
    heir_opt_flags = [
        "--mlir-to-secret-arithmetic",
        "--secret-insert-mgmt-bfv",
        "--validate-noise=model=bfv-noise-symbol annotate-noise-bound=true",
        "--secret-distribute-generic",
        "--secret-to-bgv",
        "--lwe-add-client-interface",
        "--scheme-to-openfhe=insert-debug-handler-calls=true",
    ],
    mlir_src = "mult_dep_8.mlir",
    tags = ["notap"],
    test_src = "mult_dep_8_debug_test.cpp",
)
