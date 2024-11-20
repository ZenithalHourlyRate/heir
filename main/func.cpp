
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
  const auto& v10 = v0->Relinearize(v9);
  EvalNoiseBGV(v0, secretKey, v10, "57.96", "v10=Relinearize v9");
  const auto& v11 = v0->EvalMultNoRelin(v3, v4);
  EvalNoiseBGV(v0, secretKey, v11, "57.96", "v11=EvalMultNoRelin v3, v4");
  const auto& v12 = v0->Relinearize(v11);
  EvalNoiseBGV(v0, secretKey, v12, "57.96", "v12=Relinearize v11");
  const auto& v13 = v0->EvalMultNoRelin(v10, v12);
  EvalNoiseBGV(v0, secretKey, v13, "120.88", "v13=EvalMultNoRelin v10, v12");
  const auto& v14 = v0->ModReduce(v13);
  EvalNoiseBGV(v0, secretKey, v14, "65.88", "v14=ModReduce v13");
  const auto& v15 = v0->Relinearize(v14);
  EvalNoiseBGV(v0, secretKey, v15, "65.88", "v15=Relinearize v14");
  const auto& v16 = v0->EvalMultNoRelin(v5, v6);
  EvalNoiseBGV(v0, secretKey, v16, "57.96", "v16=EvalMultNoRelin v5, v6");
  const auto& v17 = v0->Relinearize(v16);
  EvalNoiseBGV(v0, secretKey, v17, "57.96", "v17=Relinearize v16");
  const auto& v18 = v0->ModReduce(v17);
  EvalNoiseBGV(v0, secretKey, v18, "22.82", "v18=ModReduce v17");
  const auto& v19 = v0->EvalMultNoRelin(v7, v8);
  EvalNoiseBGV(v0, secretKey, v19, "57.96", "v19=EvalMultNoRelin v7, v8");
  const auto& v20 = v0->Relinearize(v19);
  EvalNoiseBGV(v0, secretKey, v20, "57.96", "v20=Relinearize v19");
  const auto& v21 = v0->ModReduce(v20);
  EvalNoiseBGV(v0, secretKey, v21, "22.82", "v21=ModReduce v20");
  const auto& v22 = v0->EvalMultNoRelin(v18, v21);
  EvalNoiseBGV(v0, secretKey, v22, "50.23", "v22=EvalMultNoRelin v18, v21");
  const auto& v23 = v0->Relinearize(v22);
  EvalNoiseBGV(v0, secretKey, v23, "56.08", "v23=Relinearize v22");
  const auto& v24 = v0->EvalMultNoRelin(v15, v23);
  EvalNoiseBGV(v0, secretKey, v24, "126.05", "v24=EvalMultNoRelin v15, v23");
  const auto& v25 = v0->ModReduce(v24);
  EvalNoiseBGV(v0, secretKey, v25, "70.76", "v25=ModReduce v24");
  const auto& v26 = v0->Relinearize(v25);
  EvalNoiseBGV(v0, secretKey, v26, "70.76", "v26=Relinearize v25");
  const auto& v27 = v0->ModReduce(v26);
  EvalNoiseBGV(v0, secretKey, v27, "22.82", "v27=ModReduce v26");
  return v27;
}
CiphertextT func__encrypt__arg0(PrivateKeyT secretKey, CryptoContextT v28,
                                int16_t v29, PublicKeyT v30) {
  std::vector<int16_t> v31(8, v29);
  std::vector<int64_t> v32(std::begin(v31), std::end(v31));
  const auto& v33 = v28->MakePackedPlaintext(v32);
  const auto& v34 = v28->Encrypt(v30, v33);
  EvalNoiseBGV(v28, secretKey, v34, "26.5", "v34=Encrypt v30, v33");
  return v34;
}
CiphertextT func__encrypt__arg1(PrivateKeyT secretKey, CryptoContextT v35,
                                int16_t v36, PublicKeyT v37) {
  std::vector<int16_t> v38(8, v36);
  std::vector<int64_t> v39(std::begin(v38), std::end(v38));
  const auto& v40 = v35->MakePackedPlaintext(v39);
  const auto& v41 = v35->Encrypt(v37, v40);
  EvalNoiseBGV(v35, secretKey, v41, "26.5", "v41=Encrypt v37, v40");
  return v41;
}
CiphertextT func__encrypt__arg2(PrivateKeyT secretKey, CryptoContextT v42,
                                int16_t v43, PublicKeyT v44) {
  std::vector<int16_t> v45(8, v43);
  std::vector<int64_t> v46(std::begin(v45), std::end(v45));
  const auto& v47 = v42->MakePackedPlaintext(v46);
  const auto& v48 = v42->Encrypt(v44, v47);
  EvalNoiseBGV(v42, secretKey, v48, "26.5", "v48=Encrypt v44, v47");
  return v48;
}
CiphertextT func__encrypt__arg3(PrivateKeyT secretKey, CryptoContextT v49,
                                int16_t v50, PublicKeyT v51) {
  std::vector<int16_t> v52(8, v50);
  std::vector<int64_t> v53(std::begin(v52), std::end(v52));
  const auto& v54 = v49->MakePackedPlaintext(v53);
  const auto& v55 = v49->Encrypt(v51, v54);
  EvalNoiseBGV(v49, secretKey, v55, "26.5", "v55=Encrypt v51, v54");
  return v55;
}
CiphertextT func__encrypt__arg4(PrivateKeyT secretKey, CryptoContextT v56,
                                int16_t v57, PublicKeyT v58) {
  std::vector<int16_t> v59(8, v57);
  std::vector<int64_t> v60(std::begin(v59), std::end(v59));
  const auto& v61 = v56->MakePackedPlaintext(v60);
  const auto& v62 = v56->Encrypt(v58, v61);
  EvalNoiseBGV(v56, secretKey, v62, "26.5", "v62=Encrypt v58, v61");
  return v62;
}
CiphertextT func__encrypt__arg5(PrivateKeyT secretKey, CryptoContextT v63,
                                int16_t v64, PublicKeyT v65) {
  std::vector<int16_t> v66(8, v64);
  std::vector<int64_t> v67(std::begin(v66), std::end(v66));
  const auto& v68 = v63->MakePackedPlaintext(v67);
  const auto& v69 = v63->Encrypt(v65, v68);
  EvalNoiseBGV(v63, secretKey, v69, "26.5", "v69=Encrypt v65, v68");
  return v69;
}
CiphertextT func__encrypt__arg6(PrivateKeyT secretKey, CryptoContextT v70,
                                int16_t v71, PublicKeyT v72) {
  std::vector<int16_t> v73(8, v71);
  std::vector<int64_t> v74(std::begin(v73), std::end(v73));
  const auto& v75 = v70->MakePackedPlaintext(v74);
  const auto& v76 = v70->Encrypt(v72, v75);
  EvalNoiseBGV(v70, secretKey, v76, "26.5", "v76=Encrypt v72, v75");
  return v76;
}
CiphertextT func__encrypt__arg7(PrivateKeyT secretKey, CryptoContextT v77,
                                int16_t v78, PublicKeyT v79) {
  std::vector<int16_t> v80(8, v78);
  std::vector<int64_t> v81(std::begin(v80), std::end(v80));
  const auto& v82 = v77->MakePackedPlaintext(v81);
  const auto& v83 = v77->Encrypt(v79, v82);
  EvalNoiseBGV(v77, secretKey, v83, "26.5", "v83=Encrypt v79, v82");
  return v83;
}
int16_t func__decrypt__result0(PrivateKeyT secretKey, CryptoContextT v84,
                               CiphertextT v85, PrivateKeyT v86) {
  PlaintextT v87;
  v84->Decrypt(v86, v85, &v87);
  int16_t v88 = v87->GetPackedValue()[0];
  return v88;
}
CryptoContextT func__generate_crypto_context() {
  CCParamsT v89;
  v89.SetMultiplicativeDepth(3);
  v89.SetPlaintextModulus(65537);
  v89.SetSecurityLevel(HEStd_NotSet);
  v89.SetRingDim(8192);
  v89.SetMaxRelinSkDeg(2);
  v89.SetScalingTechnique(FIXEDMANUAL);
  v89.SetScalingModSize(55);
  v89.SetKeySwitchTechnique(BV);
  v89.SetDigitSize(30);
  v89.SetNumLargeDigits(0);
  CryptoContextT v90 = GenCryptoContext(v89);
  v90->Enable(PKE);
  v90->Enable(KEYSWITCH);
  v90->Enable(LEVELEDSHE);
  return v90;
}
CryptoContextT func__configure_crypto_context(PrivateKeyT secretKey,
                                              CryptoContextT v91,
                                              PrivateKeyT v92) {
  v91->EvalMultKeyGen(v92);
  return v91;
}
