
#include "src/pke/include/openfhe.h"  // from @openfhe

using namespace lbcrypto;
using CiphertextT = ConstCiphertext<DCRTPoly>;
using CCParamsT = CCParams<CryptoContextBGVRNS>;
using CryptoContextT = CryptoContext<DCRTPoly>;
using EvalKeyT = EvalKey<DCRTPoly>;
using PlaintextT = Plaintext;
using PrivateKeyT = PrivateKey<DCRTPoly>;
using PublicKeyT = PublicKey<DCRTPoly>;
void EvalNoiseBGV(CryptoContext<DCRTPoly> cryptoContext,
                  PrivateKey<DCRTPoly> privateKey,
                  ConstCiphertext<DCRTPoly> ciphertext, std::string tag);
CiphertextT func(PrivateKeyT secretKey, CryptoContextT v0, CiphertextT v1,
                 CiphertextT v2) {
  std::vector<int64_t> v3 = {0, 0, 0, 0, 0, 0, 0, 1};
  const auto& v4 = v0->EvalMultNoRelin(v1, v2);
  EvalNoiseBGV(v0, secretKey, v4, "v4=EvalMultNoRelin v1, v2");
  const auto& v5 = v0->Relinearize(v4);
  EvalNoiseBGV(v0, secretKey, v5, "v5=Relinearize v4");
  const auto& v6 = v0->EvalRotate(v5, 4);
  EvalNoiseBGV(v0, secretKey, v5, "EvalRotate v5, 4");
  const auto& v7 = v0->EvalAdd(v5, v6);
  EvalNoiseBGV(v0, secretKey, v7, "v7=EvalAdd v5, v6");
  const auto& v8 = v0->EvalRotate(v7, 2);
  EvalNoiseBGV(v0, secretKey, v7, "EvalRotate v7, 2");
  const auto& v9 = v0->EvalAdd(v7, v8);
  EvalNoiseBGV(v0, secretKey, v9, "v9=EvalAdd v7, v8");
  const auto& v10 = v0->EvalRotate(v9, 1);
  EvalNoiseBGV(v0, secretKey, v9, "EvalRotate v9, 1");
  const auto& v11 = v0->EvalAdd(v9, v10);
  EvalNoiseBGV(v0, secretKey, v11, "v11=EvalAdd v9, v10");
  const auto& v12 = v0->MakePackedPlaintext(v3);
  const auto& v13 = v0->EvalMult(v11, v12);
  EvalNoiseBGV(v0, secretKey, v13, "v13=EvalMult v11, v12");
  const auto& v14 = v0->EvalRotate(v13, 7);
  EvalNoiseBGV(v0, secretKey, v13, "EvalRotate v13, 7");
  const auto& v15 = v14;
  return v15;
}
CiphertextT func__encrypt__arg0(PrivateKeyT secretKey, CryptoContextT v16,
                                std::vector<int16_t> v17, PublicKeyT v18) {
  std::vector<int64_t> v19(std::begin(v17), std::end(v17));
  const auto& v20 = v16->MakePackedPlaintext(v19);
  const auto& v21 = v16->Encrypt(v18, v20);
  EvalNoiseBGV(v16, secretKey, v21, "v21=Encrypt v18, v20");
  return v21;
}
CiphertextT func__encrypt__arg1(PrivateKeyT secretKey, CryptoContextT v22,
                                std::vector<int16_t> v23, PublicKeyT v24) {
  std::vector<int64_t> v25(std::begin(v23), std::end(v23));
  const auto& v26 = v22->MakePackedPlaintext(v25);
  const auto& v27 = v22->Encrypt(v24, v26);
  EvalNoiseBGV(v22, secretKey, v27, "v27=Encrypt v24, v26");
  return v27;
}
int16_t func__decrypt__result0(PrivateKeyT secretKey, CryptoContextT v28,
                               CiphertextT v29, PrivateKeyT v30) {
  PlaintextT v31;
  v28->Decrypt(v30, v29, &v31);
  int16_t v32 = v31->GetPackedValue()[0];
  return v32;
}
CryptoContextT func__generate_crypto_context() {
  CCParamsT v33;
  v33.SetMultiplicativeDepth(2);
  v33.SetPlaintextModulus(4295294977);
  v33.SetKeySwitchTechnique(BV);
  v33.SetDigitSize(2);
  CryptoContextT v34 = GenCryptoContext(v33);
  v34->Enable(PKE);
  v34->Enable(KEYSWITCH);
  v34->Enable(LEVELEDSHE);
  return v34;
}
CryptoContextT func__configure_crypto_context(PrivateKeyT secretKey,
                                              CryptoContextT v35,
                                              PrivateKeyT v36) {
  v35->EvalMultKeyGen(v36);
  v35->EvalRotateKeyGen(v36, {1, 2, 4, 7});
  return v35;
}
