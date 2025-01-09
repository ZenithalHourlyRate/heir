bazel run //tools:heir-opt -- --secretize --mlir-to-secret-arithmetic --operation-balancer --secret-insert-mgmt-bgv $PWD/func.mlir > func_middle.mlir
bazel run //tools:heir-opt -- --secretize --mlir-to-openfhe-bgv='entry-function=func ciphertext-degree=8' $PWD/func.mlir > func_output.mlir
bazel run //tools:heir-translate -- --emit-openfhe-pke-header --openfhe-include-type=source-relative $PWD/func_output.mlir > func.h
bazel run //tools:heir-translate -- --emit-openfhe-pke --openfhe-include-type=source-relative $PWD/func_output.mlir > func.cpp
sed -i 's/(CryptoContextT/(PrivateKeyT secretKey, CryptoContextT/g' func.h func.cpp
sed -i 's/<<NULL ATTRIBUTE>>/"26.5"/g' func.cpp
sed -i '12s/^/void EvalNoiseBGV(CryptoContext<DCRTPoly> &cryptoContext, PrivateKey<DCRTPoly> privateKey, ConstCiphertext<DCRTPoly> ciphertext, std::string bound, std::string tag);/g' func.cpp
bazel run //example_bgv:main
