
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
  EvalNoiseBGV(v0, secretKey, v9, "58.46", "v9=EvalMultNoRelin v1, v1");
  const auto& v10 = v0->ModReduce(v9);
  EvalNoiseBGV(v0, secretKey, v10, "30.39", "v10=ModReduce v9");
  const auto& v11 = v0->Relinearize(v10);
  EvalNoiseBGV(v0, secretKey, v11, "56.08", "v11=Relinearize v10");
  const auto& v12 = v0->EvalMultNoRelin(v11, v11);
  EvalNoiseBGV(v0, secretKey, v12, "121.79", "v12=EvalMultNoRelin v11, v11");
  const auto& v13 = v0->ModReduce(v12);
  EvalNoiseBGV(v0, secretKey, v13, "66.21", "v13=ModReduce v12");
  const auto& v14 = v0->Relinearize(v13);
  EvalNoiseBGV(v0, secretKey, v14, "66.21", "v14=Relinearize v13");
  const auto& v15 = v0->EvalMultNoRelin(v1, v1);
  EvalNoiseBGV(v0, secretKey, v15, "58.46", "v15=EvalMultNoRelin v1, v1");
  const auto& v16 = v0->Relinearize(v15);
  EvalNoiseBGV(v0, secretKey, v16, "58.46", "v16=Relinearize v15");
  const auto& v17 = v0->ModReduce(v16);
  EvalNoiseBGV(v0, secretKey, v17, "23.32", "v17=ModReduce v16");
  const auto& v18 = v0->EvalMultNoRelin(v11, v17);
  EvalNoiseBGV(v0, secretKey, v18, "86.98", "v18=EvalMultNoRelin v11, v17");
  const auto& v19 = v0->Relinearize(v18);
  EvalNoiseBGV(v0, secretKey, v19, "86.98", "v19=Relinearize v18");
  const auto& v20 = v0->ModReduce(v19);
  EvalNoiseBGV(v0, secretKey, v20, "31.69", "v20=ModReduce v19");
  const auto& v21 = v0->EvalMultNoRelin(v14, v20);
  EvalNoiseBGV(v0, secretKey, v21, "106.10", "v21=EvalMultNoRelin v14, v20");
  const auto& v22 = v0->Relinearize(v21);
  EvalNoiseBGV(v0, secretKey, v22, "106.10", "v22=Relinearize v21");
  const auto& v23 = v0->ModReduce(v22);
  EvalNoiseBGV(v0, secretKey, v23, "49.60", "v23=ModReduce v22");
  return v23;
}
CiphertextT func__encrypt__arg0(PrivateKeyT secretKey, CryptoContextT v24,
                                int16_t v25, PublicKeyT v26) {
  std::vector<int16_t> v27(8, v25);
  std::vector<int64_t> v28(std::begin(v27), std::end(v27));
  const auto& v29 = v24->MakePackedPlaintext(v28);
  const auto& v30 = v24->Encrypt(v26, v29);
  EvalNoiseBGV(v24, secretKey, v30, "26.5", "v30=Encrypt v26, v29");
  return v30;
}
CiphertextT func__encrypt__arg1(PrivateKeyT secretKey, CryptoContextT v31,
                                int16_t v32, PublicKeyT v33) {
  std::vector<int16_t> v34(8, v32);
  std::vector<int64_t> v35(std::begin(v34), std::end(v34));
  const auto& v36 = v31->MakePackedPlaintext(v35);
  const auto& v37 = v31->Encrypt(v33, v36);
  EvalNoiseBGV(v31, secretKey, v37, "26.5", "v37=Encrypt v33, v36");
  return v37;
}
CiphertextT func__encrypt__arg2(PrivateKeyT secretKey, CryptoContextT v38,
                                int16_t v39, PublicKeyT v40) {
  std::vector<int16_t> v41(8, v39);
  std::vector<int64_t> v42(std::begin(v41), std::end(v41));
  const auto& v43 = v38->MakePackedPlaintext(v42);
  const auto& v44 = v38->Encrypt(v40, v43);
  EvalNoiseBGV(v38, secretKey, v44, "26.5", "v44=Encrypt v40, v43");
  return v44;
}
CiphertextT func__encrypt__arg3(PrivateKeyT secretKey, CryptoContextT v45,
                                int16_t v46, PublicKeyT v47) {
  std::vector<int16_t> v48(8, v46);
  std::vector<int64_t> v49(std::begin(v48), std::end(v48));
  const auto& v50 = v45->MakePackedPlaintext(v49);
  const auto& v51 = v45->Encrypt(v47, v50);
  EvalNoiseBGV(v45, secretKey, v51, "26.5", "v51=Encrypt v47, v50");
  return v51;
}
CiphertextT func__encrypt__arg4(PrivateKeyT secretKey, CryptoContextT v52,
                                int16_t v53, PublicKeyT v54) {
  std::vector<int16_t> v55(8, v53);
  std::vector<int64_t> v56(std::begin(v55), std::end(v55));
  const auto& v57 = v52->MakePackedPlaintext(v56);
  const auto& v58 = v52->Encrypt(v54, v57);
  EvalNoiseBGV(v52, secretKey, v58, "26.5", "v58=Encrypt v54, v57");
  return v58;
}
CiphertextT func__encrypt__arg5(PrivateKeyT secretKey, CryptoContextT v59,
                                int16_t v60, PublicKeyT v61) {
  std::vector<int16_t> v62(8, v60);
  std::vector<int64_t> v63(std::begin(v62), std::end(v62));
  const auto& v64 = v59->MakePackedPlaintext(v63);
  const auto& v65 = v59->Encrypt(v61, v64);
  EvalNoiseBGV(v59, secretKey, v65, "26.5", "v65=Encrypt v61, v64");
  return v65;
}
CiphertextT func__encrypt__arg6(PrivateKeyT secretKey, CryptoContextT v66,
                                int16_t v67, PublicKeyT v68) {
  std::vector<int16_t> v69(8, v67);
  std::vector<int64_t> v70(std::begin(v69), std::end(v69));
  const auto& v71 = v66->MakePackedPlaintext(v70);
  const auto& v72 = v66->Encrypt(v68, v71);
  EvalNoiseBGV(v66, secretKey, v72, "26.5", "v72=Encrypt v68, v71");
  return v72;
}
CiphertextT func__encrypt__arg7(PrivateKeyT secretKey, CryptoContextT v73,
                                int16_t v74, PublicKeyT v75) {
  std::vector<int16_t> v76(8, v74);
  std::vector<int64_t> v77(std::begin(v76), std::end(v76));
  const auto& v78 = v73->MakePackedPlaintext(v77);
  const auto& v79 = v73->Encrypt(v75, v78);
  EvalNoiseBGV(v73, secretKey, v79, "26.5", "v79=Encrypt v75, v78");
  return v79;
}
int16_t func__decrypt__result0(PrivateKeyT secretKey, CryptoContextT v80,
                               CiphertextT v81, PrivateKeyT v82) {
  PlaintextT v83;
  v80->Decrypt(v82, v81, &v83);
  int16_t v84 = v83->GetPackedValue()[0];
  return v84;
}
CryptoContextT func__generate_crypto_context() {
  CCParamsT v85;
  v85.SetMultiplicativeDepth(3);
  v85.SetPlaintextModulus(65537);
  v85.SetSecurityLevel(HEStd_NotSet);
  v85.SetRingDim(8192);
  v85.SetMaxRelinSkDeg(2);
  v85.SetScalingTechnique(FIXEDMANUAL);
  v85.SetScalingModSize(55);
  v85.SetKeySwitchTechnique(BV);
  v85.SetDigitSize(30);
  v85.SetNumLargeDigits(0);
  CryptoContextT v86 = GenCryptoContext(v85);
  v86->Enable(PKE);
  v86->Enable(KEYSWITCH);
  v86->Enable(LEVELEDSHE);
  return v86;
}
CryptoContextT func__configure_crypto_context(PrivateKeyT secretKey,
                                              CryptoContextT v87,
                                              PrivateKeyT v88) {
  v87->EvalMultKeyGen(v88);
  return v87;
}
