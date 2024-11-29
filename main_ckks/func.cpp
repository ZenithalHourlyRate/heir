
#include "src/pke/include/openfhe.h"  // from @openfhe

using namespace lbcrypto;
using CiphertextT = ConstCiphertext<DCRTPoly>;
using CCParamsT = CCParams<CryptoContextCKKSRNS>;
using CryptoContextT = CryptoContext<DCRTPoly>;
using EvalKeyT = EvalKey<DCRTPoly>;
using PlaintextT = Plaintext;
using PrivateKeyT = PrivateKey<DCRTPoly>;
using PublicKeyT = PublicKey<DCRTPoly>;
void EvalNoiseCKKS(CryptoContext<DCRTPoly> cryptoContext,
                   PrivateKey<DCRTPoly> privateKey,
                   ConstCiphertext<DCRTPoly> ciphertext, std::string bound,
                   std::string tag);
CiphertextT func(PrivateKeyT secretKey, CryptoContextT v0, CiphertextT v1,
                 CiphertextT v2) {
  std::vector<double> v3 = {0.000000e+00, 0.000000e+00, 0.000000e+00,
                            0.000000e+00, 0.000000e+00, 0.000000e+00,
                            0.000000e+00, 1.000000e+00};
  std::vector<double> v4(8, 0.0999755859375);
  const auto& v5 = v0->EvalMultNoRelin(v1, v2);
  EvalNoiseCKKS(v0, secretKey, v5, "26.5", "v5=EvalMultNoRelin v1, v2");
  const auto& v6 = v0->Relinearize(v5);
  EvalNoiseCKKS(v0, secretKey, v6, "26.5", "v6=Relinearize v5");
  auto v4_filled_n =
      v0->GetCryptoParameters()->GetElementParams()->GetRingDimension() / 2;
  auto v4_filled = v4;
  v4_filled.clear();
  v4_filled.reserve(v4_filled_n);
  for (auto i = 0; i < v4_filled_n; ++i) {
    v4_filled.push_back(v4[i % v4.size()]);
  }
  const auto& v7 = v0->MakeCKKSPackedPlaintext(v4_filled);
  const auto& v8 = v0->EvalAdd(v6, v7);
  EvalNoiseCKKS(v0, secretKey, v8, "26.5", "v8=EvalAdd v6, v7");
  const auto& v9 = v0->EvalRotate(v8, 6);
  EvalNoiseCKKS(v0, secretKey, v9, "26.5", "v9=EvalRotate v8, 6");
  const auto& v10 = v0->EvalRotate(v6, 7);
  EvalNoiseCKKS(v0, secretKey, v10, "26.5", "v10=EvalRotate v6, 7");
  const auto& v11 = v0->EvalAdd(v9, v10);
  EvalNoiseCKKS(v0, secretKey, v11, "26.5", "v11=EvalAdd v9, v10");
  const auto& v12 = v0->EvalAdd(v11, v6);
  EvalNoiseCKKS(v0, secretKey, v12, "26.5", "v12=EvalAdd v11, v6");
  const auto& v13 = v0->EvalRotate(v12, 6);
  EvalNoiseCKKS(v0, secretKey, v13, "26.5", "v13=EvalRotate v12, 6");
  const auto& v14 = v0->EvalAdd(v13, v10);
  EvalNoiseCKKS(v0, secretKey, v14, "26.5", "v14=EvalAdd v13, v10");
  const auto& v15 = v0->EvalAdd(v14, v6);
  EvalNoiseCKKS(v0, secretKey, v15, "26.5", "v15=EvalAdd v14, v6");
  const auto& v16 = v0->EvalRotate(v15, 6);
  EvalNoiseCKKS(v0, secretKey, v16, "26.5", "v16=EvalRotate v15, 6");
  const auto& v17 = v0->EvalAdd(v16, v10);
  EvalNoiseCKKS(v0, secretKey, v17, "26.5", "v17=EvalAdd v16, v10");
  const auto& v18 = v0->EvalAdd(v17, v6);
  EvalNoiseCKKS(v0, secretKey, v18, "26.5", "v18=EvalAdd v17, v6");
  const auto& v19 = v0->EvalRotate(v18, 7);
  EvalNoiseCKKS(v0, secretKey, v19, "26.5", "v19=EvalRotate v18, 7");
  const auto& v20 = v0->EvalAdd(v19, v6);
  EvalNoiseCKKS(v0, secretKey, v20, "26.5", "v20=EvalAdd v19, v6");
  auto v3_filled_n =
      v0->GetCryptoParameters()->GetElementParams()->GetRingDimension() / 2;
  auto v3_filled = v3;
  v3_filled.clear();
  v3_filled.reserve(v3_filled_n);
  for (auto i = 0; i < v3_filled_n; ++i) {
    v3_filled.push_back(v3[i % v3.size()]);
  }
  const auto& v21 = v0->MakeCKKSPackedPlaintext(v3_filled);
  const auto& v22 = v0->EvalMult(v20, v21);
  EvalNoiseCKKS(v0, secretKey, v22, "26.5", "v22=EvalMult v20, v21");
  const auto& v23 = v0->EvalRotate(v22, 7);
  EvalNoiseCKKS(v0, secretKey, v23, "26.5", "v23=EvalRotate v22, 7");
  const auto& v24 = v23;
  return v24;
}
CryptoContextT func__generate_crypto_context() {
  CCParamsT v25;
  v25.SetMultiplicativeDepth(2);
  v25.SetPlaintextModulus(4295294977);
  v25.SetExecutionMode(EXEC_NOISE_ESTIMATION);
  CryptoContextT v26 = GenCryptoContext(v25);
  v26->Enable(PKE);
  v26->Enable(KEYSWITCH);
  v26->Enable(LEVELEDSHE);
  return v26;
}
CryptoContextT func__configure_crypto_context(PrivateKeyT secretKey,
                                              CryptoContextT v27,
                                              PrivateKeyT v28) {
  v27->EvalMultKeyGen(v28);
  v27->EvalRotateKeyGen(v28, {6, 7});
  return v27;
}
