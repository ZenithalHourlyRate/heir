# A library build target that encapsulates the HEIR-generated code.
cc_library(
    name = "func",
    srcs = ["func.cpp"],
    hdrs = ["func.h"],
    deps = ["@openfhe//:pke"],
)

# An executable build target that contains your main function and links
# against the above.
cc_binary(
    name = "main",
    srcs = ["main.cpp"],
    deps = [
        ":func",
        "@openfhe//:core",
        "@openfhe//:pke",
    ],
)
