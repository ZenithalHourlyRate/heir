
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
  const auto& v9 = v0->EvalMultNoRelin(v1, v2);
  EvalNoiseBGV(v0, secretKey, v9, "58.17 M(s 2)", "v9=EvalMultNoRelin v1, v2");
  const auto& v10 = v0->ModReduce(v9);
  EvalNoiseBGV(v0, secretKey, v10, "29.53 M(s 2)", "v10=ModReduce v9");
  const auto& v11 = v0->Relinearize(v10);
  EvalNoiseBGV(v0, secretKey, v11, "56.08 M(s 2)", "v11=Relinearize v10");
  const auto& v12 = v0->ModReduce(v11);
  EvalNoiseBGV(v0, secretKey, v12, "22.82 M(s 1)", "v12=ModReduce v11");
  const auto& v13 = v0->EvalMultNoRelin(v3, v4);
  EvalNoiseBGV(v0, secretKey, v13, "58.17 M(s 2)",
               "v13=EvalMultNoRelin v3, v4");
  const auto& v14 = v0->ModReduce(v13);
  EvalNoiseBGV(v0, secretKey, v14, "29.53 M(s 2)", "v14=ModReduce v13");
  const auto& v15 = v0->Relinearize(v14);
  EvalNoiseBGV(v0, secretKey, v15, "56.08 M(s 2)", "v15=Relinearize v14");
  const auto& v16 = v0->ModReduce(v15);
  EvalNoiseBGV(v0, secretKey, v16, "22.82 M(s 1)", "v16=ModReduce v15");
  const auto& v17 = v0->EvalMultNoRelin(v12, v16);
  EvalNoiseBGV(v0, secretKey, v17, "50.23 M(s 2)",
               "v17=EvalMultNoRelin v12, v16");
  const auto& v18 = v0->Relinearize(v17);
  EvalNoiseBGV(v0, secretKey, v18, "55.79 M(s 2)", "v18=Relinearize v17");
  const auto& v19 = v0->EvalMultNoRelin(v5, v6);
  EvalNoiseBGV(v0, secretKey, v19, "58.17 M(s 2)",
               "v19=EvalMultNoRelin v5, v6");
  const auto& v20 = v0->ModReduce(v19);
  EvalNoiseBGV(v0, secretKey, v20, "29.53 M(s 2)", "v20=ModReduce v19");
  const auto& v21 = v0->Relinearize(v20);
  EvalNoiseBGV(v0, secretKey, v21, "56.08 M(s 2)", "v21=Relinearize v20");
  const auto& v22 = v0->EvalMultNoRelin(v7, v8);
  EvalNoiseBGV(v0, secretKey, v22, "58.17 M(s 2)",
               "v22=EvalMultNoRelin v7, v8");
  const auto& v23 = v0->Relinearize(v22);
  EvalNoiseBGV(v0, secretKey, v23, "58.23 M(s 2)", "v23=Relinearize v22");
  const auto& v24 = v0->ModReduce(v23);
  EvalNoiseBGV(v0, secretKey, v24, "22.82 M(s 1)", "v24=ModReduce v23");
  const auto& v25 = v0->EvalMultNoRelin(v21, v24);
  EvalNoiseBGV(v0, secretKey, v25, "84.45 M(s 3)",
               "v25=EvalMultNoRelin v21, v24");
  const auto& v26 = v0->Relinearize(v25);
  EvalNoiseBGV(v0, secretKey, v26, "84.45 M(s 3)", "v26=Relinearize v25");
  const auto& v27 = v0->ModReduce(v26);
  EvalNoiseBGV(v0, secretKey, v27, "31.45 M(s 3)", "v27=ModReduce v26");
  const auto& v28 = v0->EvalMultNoRelin(v18, v27);
  EvalNoiseBGV(v0, secretKey, v28, "93.82 M(s 5)",
               "v28=EvalMultNoRelin v18, v27");
  const auto& v29 = v0->Relinearize(v28);
  EvalNoiseBGV(v0, secretKey, v29, "93.82 M(s 5)", "v29=Relinearize v28");
  const auto& v30 = v0->ModReduce(v29);
  EvalNoiseBGV(v0, secretKey, v30, "40.82 M(s 5)", "v30=ModReduce v29");
  return v30;
}
CiphertextT func__encrypt__arg0(PrivateKeyT secretKey, CryptoContextT v31,
                                int16_t v32, PublicKeyT v33) {
  std::vector<int16_t> v34(8, v32);
  std::vector<int64_t> v35(std::begin(v34), std::end(v34));
  const auto& v36 = v31->MakePackedPlaintext(v35);
  const auto& v37 = v31->Encrypt(v33, v36);
  EvalNoiseBGV(v31, secretKey, v37, "26.5", "v37=Encrypt v33, v36");
  return v37;
}
CiphertextT func__encrypt__arg1(PrivateKeyT secretKey, CryptoContextT v38,
                                int16_t v39, PublicKeyT v40) {
  std::vector<int16_t> v41(8, v39);
  std::vector<int64_t> v42(std::begin(v41), std::end(v41));
  const auto& v43 = v38->MakePackedPlaintext(v42);
  const auto& v44 = v38->Encrypt(v40, v43);
  EvalNoiseBGV(v38, secretKey, v44, "26.5", "v44=Encrypt v40, v43");
  return v44;
}
CiphertextT func__encrypt__arg2(PrivateKeyT secretKey, CryptoContextT v45,
                                int16_t v46, PublicKeyT v47) {
  std::vector<int16_t> v48(8, v46);
  std::vector<int64_t> v49(std::begin(v48), std::end(v48));
  const auto& v50 = v45->MakePackedPlaintext(v49);
  const auto& v51 = v45->Encrypt(v47, v50);
  EvalNoiseBGV(v45, secretKey, v51, "26.5", "v51=Encrypt v47, v50");
  return v51;
}
CiphertextT func__encrypt__arg3(PrivateKeyT secretKey, CryptoContextT v52,
                                int16_t v53, PublicKeyT v54) {
  std::vector<int16_t> v55(8, v53);
  std::vector<int64_t> v56(std::begin(v55), std::end(v55));
  const auto& v57 = v52->MakePackedPlaintext(v56);
  const auto& v58 = v52->Encrypt(v54, v57);
  EvalNoiseBGV(v52, secretKey, v58, "26.5", "v58=Encrypt v54, v57");
  return v58;
}
CiphertextT func__encrypt__arg4(PrivateKeyT secretKey, CryptoContextT v59,
                                int16_t v60, PublicKeyT v61) {
  std::vector<int16_t> v62(8, v60);
  std::vector<int64_t> v63(std::begin(v62), std::end(v62));
  const auto& v64 = v59->MakePackedPlaintext(v63);
  const auto& v65 = v59->Encrypt(v61, v64);
  EvalNoiseBGV(v59, secretKey, v65, "26.5", "v65=Encrypt v61, v64");
  return v65;
}
CiphertextT func__encrypt__arg5(PrivateKeyT secretKey, CryptoContextT v66,
                                int16_t v67, PublicKeyT v68) {
  std::vector<int16_t> v69(8, v67);
  std::vector<int64_t> v70(std::begin(v69), std::end(v69));
  const auto& v71 = v66->MakePackedPlaintext(v70);
  const auto& v72 = v66->Encrypt(v68, v71);
  EvalNoiseBGV(v66, secretKey, v72, "26.5", "v72=Encrypt v68, v71");
  return v72;
}
CiphertextT func__encrypt__arg6(PrivateKeyT secretKey, CryptoContextT v73,
                                int16_t v74, PublicKeyT v75) {
  std::vector<int16_t> v76(8, v74);
  std::vector<int64_t> v77(std::begin(v76), std::end(v76));
  const auto& v78 = v73->MakePackedPlaintext(v77);
  const auto& v79 = v73->Encrypt(v75, v78);
  EvalNoiseBGV(v73, secretKey, v79, "26.5", "v79=Encrypt v75, v78");
  return v79;
}
CiphertextT func__encrypt__arg7(PrivateKeyT secretKey, CryptoContextT v80,
                                int16_t v81, PublicKeyT v82) {
  std::vector<int16_t> v83(8, v81);
  std::vector<int64_t> v84(std::begin(v83), std::end(v83));
  const auto& v85 = v80->MakePackedPlaintext(v84);
  const auto& v86 = v80->Encrypt(v82, v85);
  EvalNoiseBGV(v80, secretKey, v86, "26.5", "v86=Encrypt v82, v85");
  return v86;
}
int16_t func__decrypt__result0(PrivateKeyT secretKey, CryptoContextT v87,
                               CiphertextT v88, PrivateKeyT v89) {
  PlaintextT v90;
  v87->Decrypt(v89, v88, &v90);
  int16_t v91 = v90->GetPackedValue()[0];
  return v91;
}
CryptoContextT func__generate_crypto_context() {
  CCParamsT v92;
  v92.SetMultiplicativeDepth(3);
  v92.SetPlaintextModulus(65537);
  v92.SetSecurityLevel(HEStd_NotSet);
  v92.SetRingDim(8192);
  v92.SetMaxRelinSkDeg(2);
  v92.SetScalingTechnique(FIXEDMANUAL);
  v92.SetScalingModSize(53);
  v92.SetKeySwitchTechnique(BV);
  v92.SetDigitSize(30);
  v92.SetNumLargeDigits(0);
  CryptoContextT v93 = GenCryptoContext(v92);
  v93->Enable(PKE);
  v93->Enable(KEYSWITCH);
  v93->Enable(LEVELEDSHE);
  return v93;
}
CryptoContextT func__configure_crypto_context(PrivateKeyT secretKey,
                                              CryptoContextT v94,
                                              PrivateKeyT v95) {
  v94->EvalMultKeyGen(v95);
  return v94;
}
