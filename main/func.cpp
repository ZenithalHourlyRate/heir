
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
                 CiphertextT v2, CiphertextT v3, CiphertextT v4, CiphertextT v5,
                 CiphertextT v6, CiphertextT v7, CiphertextT v8) {
  const auto& v9 = v0->EvalMultNoRelin(v1, v2);
  EvalNoiseBGV(v0, secretKey, v9, "v9=EvalMultNoRelin v1, v2");
  const auto& v10 = v0->Relinearize(v9);
  EvalNoiseBGV(v0, secretKey, v10, "v10=Relinearize v9");
  const auto& v11 = v0->EvalMultNoRelin(v10, v3);
  EvalNoiseBGV(v0, secretKey, v11, "v11=EvalMultNoRelin v10, v3");
  const auto& v12 = v0->ModReduce(v11);
  EvalNoiseBGV(v0, secretKey, v12, "v12=ModReduce v11");
  const auto& v13 = v0->Relinearize(v12);
  EvalNoiseBGV(v0, secretKey, v13, "v13=Relinearize v12");
  const auto& v14 = v0->EvalMultNoRelin(v4, v5);
  EvalNoiseBGV(v0, secretKey, v14, "v14=EvalMultNoRelin v4, v5");
  const auto& v15 = v0->Relinearize(v14);
  EvalNoiseBGV(v0, secretKey, v15, "v15=Relinearize v14");
  const auto& v16 = v0->EvalMultNoRelin(v15, v6);
  EvalNoiseBGV(v0, secretKey, v16, "v16=EvalMultNoRelin v15, v6");
  const auto& v17 = v0->Relinearize(v16);
  EvalNoiseBGV(v0, secretKey, v17, "v17=Relinearize v16");
  const auto& v18 = v0->ModReduce(v17);
  EvalNoiseBGV(v0, secretKey, v18, "v18=ModReduce v17");
  const auto& v19 = v0->EvalMultNoRelin(v13, v18);
  EvalNoiseBGV(v0, secretKey, v19, "v19=EvalMultNoRelin v13, v18");
  const auto& v20 = v0->Relinearize(v19);
  EvalNoiseBGV(v0, secretKey, v20, "v20=Relinearize v19");
  const auto& v21 = v0->ModReduce(v20);
  EvalNoiseBGV(v0, secretKey, v21, "v21=ModReduce v20");
  return v21;
}
CiphertextT func__encrypt__arg0(PrivateKeyT secretKey, CryptoContextT v22,
                                int16_t v23, PublicKeyT v24) {
  std::vector<int16_t> v25(8, v23);
  std::vector<int64_t> v26(std::begin(v25), std::end(v25));
  const auto& v27 = v22->MakePackedPlaintext(v26);
  const auto& v28 = v22->Encrypt(v24, v27);
  EvalNoiseBGV(v22, secretKey, v28, "v28=Encrypt v24, v27");
  return v28;
}
CiphertextT func__encrypt__arg1(PrivateKeyT secretKey, CryptoContextT v29,
                                int16_t v30, PublicKeyT v31) {
  std::vector<int16_t> v32(8, v30);
  std::vector<int64_t> v33(std::begin(v32), std::end(v32));
  const auto& v34 = v29->MakePackedPlaintext(v33);
  const auto& v35 = v29->Encrypt(v31, v34);
  EvalNoiseBGV(v29, secretKey, v35, "v35=Encrypt v31, v34");
  return v35;
}
CiphertextT func__encrypt__arg2(PrivateKeyT secretKey, CryptoContextT v36,
                                int16_t v37, PublicKeyT v38) {
  std::vector<int16_t> v39(8, v37);
  std::vector<int64_t> v40(std::begin(v39), std::end(v39));
  const auto& v41 = v36->MakePackedPlaintext(v40);
  const auto& v42 = v36->Encrypt(v38, v41);
  EvalNoiseBGV(v36, secretKey, v42, "v42=Encrypt v38, v41");
  return v42;
}
CiphertextT func__encrypt__arg3(PrivateKeyT secretKey, CryptoContextT v43,
                                int16_t v44, PublicKeyT v45) {
  std::vector<int16_t> v46(8, v44);
  std::vector<int64_t> v47(std::begin(v46), std::end(v46));
  const auto& v48 = v43->MakePackedPlaintext(v47);
  const auto& v49 = v43->Encrypt(v45, v48);
  EvalNoiseBGV(v43, secretKey, v49, "v49=Encrypt v45, v48");
  return v49;
}
CiphertextT func__encrypt__arg4(PrivateKeyT secretKey, CryptoContextT v50,
                                int16_t v51, PublicKeyT v52) {
  std::vector<int16_t> v53(8, v51);
  std::vector<int64_t> v54(std::begin(v53), std::end(v53));
  const auto& v55 = v50->MakePackedPlaintext(v54);
  const auto& v56 = v50->Encrypt(v52, v55);
  EvalNoiseBGV(v50, secretKey, v56, "v56=Encrypt v52, v55");
  return v56;
}
CiphertextT func__encrypt__arg5(PrivateKeyT secretKey, CryptoContextT v57,
                                int16_t v58, PublicKeyT v59) {
  std::vector<int16_t> v60(8, v58);
  std::vector<int64_t> v61(std::begin(v60), std::end(v60));
  const auto& v62 = v57->MakePackedPlaintext(v61);
  const auto& v63 = v57->Encrypt(v59, v62);
  EvalNoiseBGV(v57, secretKey, v63, "v63=Encrypt v59, v62");
  return v63;
}
CiphertextT func__encrypt__arg6(PrivateKeyT secretKey, CryptoContextT v64,
                                int16_t v65, PublicKeyT v66) {
  std::vector<int16_t> v67(8, v65);
  std::vector<int64_t> v68(std::begin(v67), std::end(v67));
  const auto& v69 = v64->MakePackedPlaintext(v68);
  const auto& v70 = v64->Encrypt(v66, v69);
  EvalNoiseBGV(v64, secretKey, v70, "v70=Encrypt v66, v69");
  return v70;
}
CiphertextT func__encrypt__arg7(PrivateKeyT secretKey, CryptoContextT v71,
                                int16_t v72, PublicKeyT v73) {
  std::vector<int16_t> v74(8, v72);
  std::vector<int64_t> v75(std::begin(v74), std::end(v74));
  const auto& v76 = v71->MakePackedPlaintext(v75);
  const auto& v77 = v71->Encrypt(v73, v76);
  EvalNoiseBGV(v71, secretKey, v77, "v77=Encrypt v73, v76");
  return v77;
}
int16_t func__decrypt__result0(PrivateKeyT secretKey, CryptoContextT v78,
                               CiphertextT v79, PrivateKeyT v80) {
  PlaintextT v81;
  v78->Decrypt(v80, v79, &v81);
  int16_t v82 = v81->GetPackedValue()[0];
  return v82;
}
CryptoContextT func__generate_crypto_context() {
  CCParamsT v83;
  v83.SetMultiplicativeDepth(2);
  v83.SetPlaintextModulus(65537);
  v83.SetSecurityLevel(HEStd_NotSet);
  v83.SetRingDim(8192);
  v83.SetMaxRelinSkDeg(2);
  v83.SetScalingTechnique(FIXEDMANUAL);
  v83.SetScalingModSize(55);
  v83.SetKeySwitchTechnique(BV);
  v83.SetDigitSize(30);
  v83.SetNumLargeDigits(0);
  CryptoContextT v84 = GenCryptoContext(v83);
  v84->Enable(PKE);
  v84->Enable(KEYSWITCH);
  v84->Enable(LEVELEDSHE);
  return v84;
}
CryptoContextT func__configure_crypto_context(PrivateKeyT secretKey,
                                              CryptoContextT v85,
                                              PrivateKeyT v86) {
  v85->EvalMultKeyGen(v86);
  return v85;
}
