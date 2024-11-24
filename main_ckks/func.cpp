
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
CiphertextT func(PrivateKeyT secretKey, CryptoContextT v0, CiphertextT v1) {
  const auto& v2 = v0->EvalMultNoRelin(v1, v1);
  EvalNoiseCKKS(v0, secretKey, v2, "26.5", "v2=EvalMultNoRelin v1, v1");
  const auto& v3 = v0->Relinearize(v2);
  EvalNoiseCKKS(v0, secretKey, v3, "26.5", "v3=Relinearize v2");
  const auto& v4 = v0->EvalMultNoRelin(v3, v3);
  EvalNoiseCKKS(v0, secretKey, v4, "26.5", "v4=EvalMultNoRelin v3, v3");
  const auto& v5 = v0->Relinearize(v4);
  EvalNoiseCKKS(v0, secretKey, v5, "26.5", "v5=Relinearize v4");
  const auto& v6 = v0->EvalMultNoRelin(v5, v5);
  EvalNoiseCKKS(v0, secretKey, v6, "26.5", "v6=EvalMultNoRelin v5, v5");
  const auto& v7 = v0->Relinearize(v6);
  EvalNoiseCKKS(v0, secretKey, v7, "26.5", "v7=Relinearize v6");
  return v7;
}
CryptoContextT func__generate_crypto_context() {
  CCParamsT v8;
  v8.SetMultiplicativeDepth(3);
  v8.SetPlaintextModulus(4295294977);
  v8.SetExecutionMode(EXEC_NOISE_ESTIMATION);
  CryptoContextT v9 = GenCryptoContext(v8);
  v9->Enable(PKE);
  v9->Enable(KEYSWITCH);
  v9->Enable(LEVELEDSHE);
  return v9;
}
CryptoContextT func__configure_crypto_context(PrivateKeyT secretKey,
                                              CryptoContextT v10,
                                              PrivateKeyT v11) {
  v10->EvalMultKeyGen(v11);
  return v10;
}
