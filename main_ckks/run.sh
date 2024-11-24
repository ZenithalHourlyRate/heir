TMPDIR=/run/user/2015/T/bazel
bazel --output_user_root=$TMPDIR run //tools:heir-opt -- --mlir-to-secret-arithmetic="entry-function=func" --operation-balancer --mlir-to-openfhe-ckks='entry-function=func ciphertext-degree=8' $PWD/func.mlir > func_output.mlir
bazel --output_user_root=$TMPDIR run //tools:heir-translate -- --emit-openfhe-pke-header --openfhe-scheme=ckks $PWD/func_output.mlir > func.h
bazel --output_user_root=$TMPDIR run //tools:heir-translate -- --emit-openfhe-pke --openfhe-scheme=ckks $PWD/func_output.mlir > func.cpp
sed -i 's/(CryptoContextT/(PrivateKeyT secretKey, CryptoContextT/g' func.h func.cpp
sed -i 's/<<NULL ATTRIBUTE>>/"26.5"/g' func.cpp
sed -i '12s/^/void EvalNoiseCKKS(CryptoContext<DCRTPoly> &cryptoContext, PrivateKey<DCRTPoly> privateKey, ConstCiphertext<DCRTPoly> ciphertext, std::string bound, std::string tag);/g' func.cpp
bazel --output_user_root=$TMPDIR run //main_ckks:main
