# bazel run //tools:heir-opt -- --mlir-to-secret-arithmetic="entry-function=func" --operation-balancer --secret-with-mgmt-bgv="include-first=false" $PWD/func.mlir > func_middle.mlir
bazel run //tools:heir-opt -- --openfhe-configure-crypto-context='entry-function=func' $PWD/func.mlir > func_output.mlir
bazel run //tools:heir-translate -- --emit-openfhe-pke-header --openfhe-scheme=ckks --openfhe-include-type=source-relative $PWD/func_output.mlir > func.h
bazel run //tools:heir-translate -- --emit-openfhe-pke --openfhe-scheme=ckks --openfhe-include-type=source-relative $PWD/func_output.mlir > func.cpp
# bazel run //example_ckks:main
