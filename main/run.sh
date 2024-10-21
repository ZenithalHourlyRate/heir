TMPDIR=/run/user/2015/T/bazel
bazel --output_user_root=$TMPDIR run //tools:heir-opt -- --mlir-to-openfhe-bgv='entry-function=func ciphertext-degree=8' $PWD/func.mlir > func_output.mlir
bazel --output_user_root=$TMPDIR run //tools:heir-translate -- --emit-openfhe-pke-header $PWD/func_output.mlir > func.h
bazel --output_user_root=$TMPDIR run //tools:heir-translate -- --emit-openfhe-pke $PWD/func_output.mlir > func.cpp
bazel --output_user_root=$TMPDIR run //main:main
