
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
  EvalNoiseBGV(v0, secretKey, v9, "57.96", "v9=EvalMultNoRelin v1, v2");
  const auto& v10 = v0->ModReduce(v9);
  EvalNoiseBGV(v0, secretKey, v10, "30.39", "v10=ModReduce v9");
  const auto& v11 = v0->Relinearize(v10);
  EvalNoiseBGV(v0, secretKey, v11, "56.58", "v11=Relinearize v10");
  const auto& v12 = v0->EvalMultNoRelin(v3, v4);
  EvalNoiseBGV(v0, secretKey, v12, "57.96", "v12=EvalMultNoRelin v3, v4");
  const auto& v13 = v0->ModReduce(v12);
  EvalNoiseBGV(v0, secretKey, v13, "30.39", "v13=ModReduce v12");
  const auto& v14 = v0->Relinearize(v13);
  EvalNoiseBGV(v0, secretKey, v14, "56.58", "v14=Relinearize v13");
  const auto& v15 = v0->EvalMultNoRelin(v11, v14);
  EvalNoiseBGV(v0, secretKey, v15, "120.29", "v15=EvalMultNoRelin v11, v14");
  const auto& v16 = v0->ModReduce(v15);
  EvalNoiseBGV(v0, secretKey, v16, "65.29", "v16=ModReduce v15");
  const auto& v17 = v0->Relinearize(v16);
  EvalNoiseBGV(v0, secretKey, v17, "65.29", "v17=Relinearize v16");
  const auto& v18 = v0->EvalMultNoRelin(v5, v6);
  EvalNoiseBGV(v0, secretKey, v18, "57.96", "v18=EvalMultNoRelin v5, v6");
  const auto& v19 = v0->ModReduce(v18);
  EvalNoiseBGV(v0, secretKey, v19, "30.39", "v19=ModReduce v18");
  const auto& v20 = v0->Relinearize(v19);
  EvalNoiseBGV(v0, secretKey, v20, "56.58", "v20=Relinearize v19");
  const auto& v21 = v0->EvalMultNoRelin(v7, v8);
  EvalNoiseBGV(v0, secretKey, v21, "57.96", "v21=EvalMultNoRelin v7, v8");
  const auto& v22 = v0->Relinearize(v21);
  EvalNoiseBGV(v0, secretKey, v22, "57.96", "v22=Relinearize v21");
  const auto& v23 = v0->ModReduce(v22);
  EvalNoiseBGV(v0, secretKey, v23, "23.32", "v23=ModReduce v22");
  const auto& v24 = v0->EvalMultNoRelin(v20, v23);
  EvalNoiseBGV(v0, secretKey, v24, "86.19", "v24=EvalMultNoRelin v20, v23");
  const auto& v25 = v0->Relinearize(v24);
  EvalNoiseBGV(v0, secretKey, v25, "86.19", "v25=Relinearize v24");
  const auto& v26 = v0->ModReduce(v25);
  EvalNoiseBGV(v0, secretKey, v26, "31.19", "v26=ModReduce v25");
  const auto& v27 = v0->EvalMultNoRelin(v17, v26);
  EvalNoiseBGV(v0, secretKey, v27, "101.95", "v27=EvalMultNoRelin v17, v26");
  const auto& v28 = v0->Relinearize(v27);
  EvalNoiseBGV(v0, secretKey, v28, "101.95", "v28=Relinearize v27");
  const auto& v29 = v0->ModReduce(v28);
  EvalNoiseBGV(v0, secretKey, v29, "46.95", "v29=ModReduce v28");
  return v29;
}
CiphertextT func__encrypt__arg0(PrivateKeyT secretKey, CryptoContextT v30,
                                int16_t v31, PublicKeyT v32) {
  std::vector<int16_t> v33(8, v31);
  std::vector<int64_t> v34(std::begin(v33), std::end(v33));
  const auto& v35 = v30->MakePackedPlaintext(v34);
  const auto& v36 = v30->Encrypt(v32, v35);
  EvalNoiseBGV(v30, secretKey, v36, "26.5", "v36=Encrypt v32, v35");
  return v36;
}
CiphertextT func__encrypt__arg1(PrivateKeyT secretKey, CryptoContextT v37,
                                int16_t v38, PublicKeyT v39) {
  std::vector<int16_t> v40(8, v38);
  std::vector<int64_t> v41(std::begin(v40), std::end(v40));
  const auto& v42 = v37->MakePackedPlaintext(v41);
  const auto& v43 = v37->Encrypt(v39, v42);
  EvalNoiseBGV(v37, secretKey, v43, "26.5", "v43=Encrypt v39, v42");
  return v43;
}
CiphertextT func__encrypt__arg2(PrivateKeyT secretKey, CryptoContextT v44,
                                int16_t v45, PublicKeyT v46) {
  std::vector<int16_t> v47(8, v45);
  std::vector<int64_t> v48(std::begin(v47), std::end(v47));
  const auto& v49 = v44->MakePackedPlaintext(v48);
  const auto& v50 = v44->Encrypt(v46, v49);
  EvalNoiseBGV(v44, secretKey, v50, "26.5", "v50=Encrypt v46, v49");
  return v50;
}
CiphertextT func__encrypt__arg3(PrivateKeyT secretKey, CryptoContextT v51,
                                int16_t v52, PublicKeyT v53) {
  std::vector<int16_t> v54(8, v52);
  std::vector<int64_t> v55(std::begin(v54), std::end(v54));
  const auto& v56 = v51->MakePackedPlaintext(v55);
  const auto& v57 = v51->Encrypt(v53, v56);
  EvalNoiseBGV(v51, secretKey, v57, "26.5", "v57=Encrypt v53, v56");
  return v57;
}
CiphertextT func__encrypt__arg4(PrivateKeyT secretKey, CryptoContextT v58,
                                int16_t v59, PublicKeyT v60) {
  std::vector<int16_t> v61(8, v59);
  std::vector<int64_t> v62(std::begin(v61), std::end(v61));
  const auto& v63 = v58->MakePackedPlaintext(v62);
  const auto& v64 = v58->Encrypt(v60, v63);
  EvalNoiseBGV(v58, secretKey, v64, "26.5", "v64=Encrypt v60, v63");
  return v64;
}
CiphertextT func__encrypt__arg5(PrivateKeyT secretKey, CryptoContextT v65,
                                int16_t v66, PublicKeyT v67) {
  std::vector<int16_t> v68(8, v66);
  std::vector<int64_t> v69(std::begin(v68), std::end(v68));
  const auto& v70 = v65->MakePackedPlaintext(v69);
  const auto& v71 = v65->Encrypt(v67, v70);
  EvalNoiseBGV(v65, secretKey, v71, "26.5", "v71=Encrypt v67, v70");
  return v71;
}
CiphertextT func__encrypt__arg6(PrivateKeyT secretKey, CryptoContextT v72,
                                int16_t v73, PublicKeyT v74) {
  std::vector<int16_t> v75(8, v73);
  std::vector<int64_t> v76(std::begin(v75), std::end(v75));
  const auto& v77 = v72->MakePackedPlaintext(v76);
  const auto& v78 = v72->Encrypt(v74, v77);
  EvalNoiseBGV(v72, secretKey, v78, "26.5", "v78=Encrypt v74, v77");
  return v78;
}
CiphertextT func__encrypt__arg7(PrivateKeyT secretKey, CryptoContextT v79,
                                int16_t v80, PublicKeyT v81) {
  std::vector<int16_t> v82(8, v80);
  std::vector<int64_t> v83(std::begin(v82), std::end(v82));
  const auto& v84 = v79->MakePackedPlaintext(v83);
  const auto& v85 = v79->Encrypt(v81, v84);
  EvalNoiseBGV(v79, secretKey, v85, "26.5", "v85=Encrypt v81, v84");
  return v85;
}
int16_t func__decrypt__result0(PrivateKeyT secretKey, CryptoContextT v86,
                               CiphertextT v87, PrivateKeyT v88) {
  PlaintextT v89;
  v86->Decrypt(v88, v87, &v89);
  int16_t v90 = v89->GetPackedValue()[0];
  return v90;
}
CryptoContextT func__generate_crypto_context() {
  CCParamsT v91;
  v91.SetMultiplicativeDepth(3);
  v91.SetPlaintextModulus(65537);
  v91.SetSecurityLevel(HEStd_NotSet);
  v91.SetRingDim(8192);
  v91.SetMaxRelinSkDeg(2);
  v91.SetScalingTechnique(FIXEDMANUAL);
  v91.SetScalingModSize(55);
  v91.SetKeySwitchTechnique(BV);
  v91.SetDigitSize(30);
  v91.SetNumLargeDigits(0);
  CryptoContextT v92 = GenCryptoContext(v91);
  v92->Enable(PKE);
  v92->Enable(KEYSWITCH);
  v92->Enable(LEVELEDSHE);
  return v92;
}
CryptoContextT func__configure_crypto_context(PrivateKeyT secretKey,
                                              CryptoContextT v93,
                                              PrivateKeyT v94) {
  v93->EvalMultKeyGen(v94);
  return v93;
}
