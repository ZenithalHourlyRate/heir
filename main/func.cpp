
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
                  ConstCiphertext<DCRTPoly> ciphertext, std::string bound,
                  std::string tag);
CiphertextT func(PrivateKeyT secretKey, CryptoContextT v0, CiphertextT v1,
                 CiphertextT v2, CiphertextT v3, CiphertextT v4, CiphertextT v5,
                 CiphertextT v6, CiphertextT v7, CiphertextT v8) {
  const auto& v9 = v0->EvalMultNoRelin(v1, v1);
  EvalNoiseBGV(v0, secretKey, v9, "57.67 M(s 2)", "v9=EvalMultNoRelin v1, v1");
  const auto& v10 = v0->EvalMultNoRelin(v9, v9);
  EvalNoiseBGV(v0, secretKey, v10, "119.45 M(s 2)",
               "v10=EvalMultNoRelin v9, v9");
  const auto& v11 = v0->EvalMultNoRelin(v10, v10);
  EvalNoiseBGV(v0, secretKey, v11, "242.99 M(s 2)",
               "v11=EvalMultNoRelin v10, v10");
  return v11;
}
CiphertextT func__encrypt__arg0(PrivateKeyT secretKey, CryptoContextT v12,
                                int16_t v13, PublicKeyT v14) {
  std::vector<int16_t> v15(8, v13);
  std::vector<int64_t> v16(std::begin(v15), std::end(v15));
  const auto& v17 = v12->MakePackedPlaintext(v16);
  const auto& v18 = v12->Encrypt(v14, v17);
  EvalNoiseBGV(v12, secretKey, v18, "26.5", "v18=Encrypt v14, v17");
  return v18;
}
CiphertextT func__encrypt__arg1(PrivateKeyT secretKey, CryptoContextT v19,
                                int16_t v20, PublicKeyT v21) {
  std::vector<int16_t> v22(8, v20);
  std::vector<int64_t> v23(std::begin(v22), std::end(v22));
  const auto& v24 = v19->MakePackedPlaintext(v23);
  const auto& v25 = v19->Encrypt(v21, v24);
  EvalNoiseBGV(v19, secretKey, v25, "26.5", "v25=Encrypt v21, v24");
  return v25;
}
CiphertextT func__encrypt__arg2(PrivateKeyT secretKey, CryptoContextT v26,
                                int16_t v27, PublicKeyT v28) {
  std::vector<int16_t> v29(8, v27);
  std::vector<int64_t> v30(std::begin(v29), std::end(v29));
  const auto& v31 = v26->MakePackedPlaintext(v30);
  const auto& v32 = v26->Encrypt(v28, v31);
  EvalNoiseBGV(v26, secretKey, v32, "26.5", "v32=Encrypt v28, v31");
  return v32;
}
CiphertextT func__encrypt__arg3(PrivateKeyT secretKey, CryptoContextT v33,
                                int16_t v34, PublicKeyT v35) {
  std::vector<int16_t> v36(8, v34);
  std::vector<int64_t> v37(std::begin(v36), std::end(v36));
  const auto& v38 = v33->MakePackedPlaintext(v37);
  const auto& v39 = v33->Encrypt(v35, v38);
  EvalNoiseBGV(v33, secretKey, v39, "26.5", "v39=Encrypt v35, v38");
  return v39;
}
CiphertextT func__encrypt__arg4(PrivateKeyT secretKey, CryptoContextT v40,
                                int16_t v41, PublicKeyT v42) {
  std::vector<int16_t> v43(8, v41);
  std::vector<int64_t> v44(std::begin(v43), std::end(v43));
  const auto& v45 = v40->MakePackedPlaintext(v44);
  const auto& v46 = v40->Encrypt(v42, v45);
  EvalNoiseBGV(v40, secretKey, v46, "26.5", "v46=Encrypt v42, v45");
  return v46;
}
CiphertextT func__encrypt__arg5(PrivateKeyT secretKey, CryptoContextT v47,
                                int16_t v48, PublicKeyT v49) {
  std::vector<int16_t> v50(8, v48);
  std::vector<int64_t> v51(std::begin(v50), std::end(v50));
  const auto& v52 = v47->MakePackedPlaintext(v51);
  const auto& v53 = v47->Encrypt(v49, v52);
  EvalNoiseBGV(v47, secretKey, v53, "26.5", "v53=Encrypt v49, v52");
  return v53;
}
CiphertextT func__encrypt__arg6(PrivateKeyT secretKey, CryptoContextT v54,
                                int16_t v55, PublicKeyT v56) {
  std::vector<int16_t> v57(8, v55);
  std::vector<int64_t> v58(std::begin(v57), std::end(v57));
  const auto& v59 = v54->MakePackedPlaintext(v58);
  const auto& v60 = v54->Encrypt(v56, v59);
  EvalNoiseBGV(v54, secretKey, v60, "26.5", "v60=Encrypt v56, v59");
  return v60;
}
CiphertextT func__encrypt__arg7(PrivateKeyT secretKey, CryptoContextT v61,
                                int16_t v62, PublicKeyT v63) {
  std::vector<int16_t> v64(8, v62);
  std::vector<int64_t> v65(std::begin(v64), std::end(v64));
  const auto& v66 = v61->MakePackedPlaintext(v65);
  const auto& v67 = v61->Encrypt(v63, v66);
  EvalNoiseBGV(v61, secretKey, v67, "26.5", "v67=Encrypt v63, v66");
  return v67;
}
int16_t func__decrypt__result0(PrivateKeyT secretKey, CryptoContextT v68,
                               CiphertextT v69, PrivateKeyT v70) {
  PlaintextT v71;
  v68->Decrypt(v70, v69, &v71);
  int16_t v72 = v71->GetPackedValue()[0];
  return v72;
}
CryptoContextT func__generate_crypto_context() {
  CCParamsT v73;
  v73.SetMultiplicativeDepth(6);
  v73.SetPlaintextModulus(65537);
  v73.SetSecurityLevel(HEStd_NotSet);
  v73.SetRingDim(8192);
  v73.SetMaxRelinSkDeg(2);
  v73.SetScalingTechnique(FIXEDMANUAL);
  v73.SetScalingModSize(55);
  v73.SetKeySwitchTechnique(BV);
  v73.SetDigitSize(30);
  v73.SetNumLargeDigits(0);
  CryptoContextT v74 = GenCryptoContext(v73);
  v74->Enable(PKE);
  v74->Enable(KEYSWITCH);
  v74->Enable(LEVELEDSHE);
  return v74;
}
CryptoContextT func__configure_crypto_context(PrivateKeyT secretKey,
                                              CryptoContextT v75,
                                              PrivateKeyT v76) {
  v75->EvalMultKeyGen(v76);
  return v75;
}
