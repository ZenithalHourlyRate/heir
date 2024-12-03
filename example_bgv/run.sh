bazel run //tools:heir-opt -- --mlir-to-secret-arithmetic="entry-function=func" --operation-balancer --secret-with-mgmt-bgv $PWD/func.mlir > func_middle.mlir
bazel run //tools:heir-opt -- --mlir-to-openfhe-bgv='entry-function=func ciphertext-degree=8' $PWD/func_middle.mlir > func_output.mlir
bazel run //tools:heir-translate -- --emit-openfhe-pke-header $PWD/func_output.mlir > func.h
bazel run //tools:heir-translate -- --emit-openfhe-pke $PWD/func_output.mlir > func.cpp
sed -i 's/(CryptoContextT/(PrivateKeyT secretKey, CryptoContextT/g' func.h func.cpp
sed -i 's/<<NULL ATTRIBUTE>>/"26.5"/g' func.cpp
sed -i '12s/^/void EvalNoiseBGV(CryptoContext<DCRTPoly> &cryptoContext, PrivateKey<DCRTPoly> privateKey, ConstCiphertext<DCRTPoly> ciphertext, std::string bound, std::string tag);/g' func.cpp
bazel run //example_bgv:main
