
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
  const auto& v11 = v0->EvalMultNoRelin(v3, v4);
  EvalNoiseBGV(v0, secretKey, v11, "v11=EvalMultNoRelin v3, v4");
  const auto& v12 = v0->Relinearize(v11);
  EvalNoiseBGV(v0, secretKey, v12, "v12=Relinearize v11");
  const auto& v13 = v0->EvalMultNoRelin(v10, v12);
  EvalNoiseBGV(v0, secretKey, v13, "v13=EvalMultNoRelin v10, v12");
  const auto& v14 = v0->Relinearize(v13);
  EvalNoiseBGV(v0, secretKey, v14, "v14=Relinearize v13");
  const auto& v15 = v0->EvalMultNoRelin(v5, v6);
  EvalNoiseBGV(v0, secretKey, v15, "v15=EvalMultNoRelin v5, v6");
  const auto& v16 = v0->Relinearize(v15);
  EvalNoiseBGV(v0, secretKey, v16, "v16=Relinearize v15");
  const auto& v17 = v0->EvalMultNoRelin(v7, v8);
  EvalNoiseBGV(v0, secretKey, v17, "v17=EvalMultNoRelin v7, v8");
  const auto& v18 = v0->Relinearize(v17);
  EvalNoiseBGV(v0, secretKey, v18, "v18=Relinearize v17");
  const auto& v19 = v0->EvalMultNoRelin(v16, v18);
  EvalNoiseBGV(v0, secretKey, v19, "v19=EvalMultNoRelin v16, v18");
  const auto& v20 = v0->Relinearize(v19);
  EvalNoiseBGV(v0, secretKey, v20, "v20=Relinearize v19");
  const auto& v21 = v0->EvalMultNoRelin(v14, v20);
  EvalNoiseBGV(v0, secretKey, v21, "v21=EvalMultNoRelin v14, v20");
  const auto& v22 = v0->Relinearize(v21);
  EvalNoiseBGV(v0, secretKey, v22, "v22=Relinearize v21");
  return v22;
}
CiphertextT func__encrypt__arg0(PrivateKeyT secretKey, CryptoContextT v23,
                                int16_t v24, PublicKeyT v25) {
  std::vector<int16_t> v26(8, v24);
  std::vector<int64_t> v27(std::begin(v26), std::end(v26));
  const auto& v28 = v23->MakePackedPlaintext(v27);
  const auto& v29 = v23->Encrypt(v25, v28);
  EvalNoiseBGV(v23, secretKey, v29, "v29=Encrypt v25, v28");
  return v29;
}
CiphertextT func__encrypt__arg1(PrivateKeyT secretKey, CryptoContextT v30,
                                int16_t v31, PublicKeyT v32) {
  std::vector<int16_t> v33(8, v31);
  std::vector<int64_t> v34(std::begin(v33), std::end(v33));
  const auto& v35 = v30->MakePackedPlaintext(v34);
  const auto& v36 = v30->Encrypt(v32, v35);
  EvalNoiseBGV(v30, secretKey, v36, "v36=Encrypt v32, v35");
  return v36;
}
CiphertextT func__encrypt__arg2(PrivateKeyT secretKey, CryptoContextT v37,
                                int16_t v38, PublicKeyT v39) {
  std::vector<int16_t> v40(8, v38);
  std::vector<int64_t> v41(std::begin(v40), std::end(v40));
  const auto& v42 = v37->MakePackedPlaintext(v41);
  const auto& v43 = v37->Encrypt(v39, v42);
  EvalNoiseBGV(v37, secretKey, v43, "v43=Encrypt v39, v42");
  return v43;
}
CiphertextT func__encrypt__arg3(PrivateKeyT secretKey, CryptoContextT v44,
                                int16_t v45, PublicKeyT v46) {
  std::vector<int16_t> v47(8, v45);
  std::vector<int64_t> v48(std::begin(v47), std::end(v47));
  const auto& v49 = v44->MakePackedPlaintext(v48);
  const auto& v50 = v44->Encrypt(v46, v49);
  EvalNoiseBGV(v44, secretKey, v50, "v50=Encrypt v46, v49");
  return v50;
}
CiphertextT func__encrypt__arg4(PrivateKeyT secretKey, CryptoContextT v51,
                                int16_t v52, PublicKeyT v53) {
  std::vector<int16_t> v54(8, v52);
  std::vector<int64_t> v55(std::begin(v54), std::end(v54));
  const auto& v56 = v51->MakePackedPlaintext(v55);
  const auto& v57 = v51->Encrypt(v53, v56);
  EvalNoiseBGV(v51, secretKey, v57, "v57=Encrypt v53, v56");
  return v57;
}
CiphertextT func__encrypt__arg5(PrivateKeyT secretKey, CryptoContextT v58,
                                int16_t v59, PublicKeyT v60) {
  std::vector<int16_t> v61(8, v59);
  std::vector<int64_t> v62(std::begin(v61), std::end(v61));
  const auto& v63 = v58->MakePackedPlaintext(v62);
  const auto& v64 = v58->Encrypt(v60, v63);
  EvalNoiseBGV(v58, secretKey, v64, "v64=Encrypt v60, v63");
  return v64;
}
CiphertextT func__encrypt__arg6(PrivateKeyT secretKey, CryptoContextT v65,
                                int16_t v66, PublicKeyT v67) {
  std::vector<int16_t> v68(8, v66);
  std::vector<int64_t> v69(std::begin(v68), std::end(v68));
  const auto& v70 = v65->MakePackedPlaintext(v69);
  const auto& v71 = v65->Encrypt(v67, v70);
  EvalNoiseBGV(v65, secretKey, v71, "v71=Encrypt v67, v70");
  return v71;
}
CiphertextT func__encrypt__arg7(PrivateKeyT secretKey, CryptoContextT v72,
                                int16_t v73, PublicKeyT v74) {
  std::vector<int16_t> v75(8, v73);
  std::vector<int64_t> v76(std::begin(v75), std::end(v75));
  const auto& v77 = v72->MakePackedPlaintext(v76);
  const auto& v78 = v72->Encrypt(v74, v77);
  EvalNoiseBGV(v72, secretKey, v78, "v78=Encrypt v74, v77");
  return v78;
}
int16_t func__decrypt__result0(PrivateKeyT secretKey, CryptoContextT v79,
                               CiphertextT v80, PrivateKeyT v81) {
  PlaintextT v82;
  v79->Decrypt(v81, v80, &v82);
  int16_t v83 = v82->GetPackedValue()[0];
  return v83;
}
CryptoContextT func__generate_crypto_context() {
  CCParamsT v84;
  v84.SetMultiplicativeDepth(3);
  v84.SetPlaintextModulus(65537);
  CryptoContextT v85 = GenCryptoContext(v84);
  v85->Enable(PKE);
  v85->Enable(KEYSWITCH);
  v85->Enable(LEVELEDSHE);
  return v85;
}
CryptoContextT func__configure_crypto_context(PrivateKeyT secretKey,
                                              CryptoContextT v86,
                                              PrivateKeyT v87) {
  v86->EvalMultKeyGen(v87);
  return v86;
}
