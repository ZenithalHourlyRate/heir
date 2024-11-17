
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
                 CiphertextT v6, CiphertextT v7, CiphertextT v8, CiphertextT v9,
                 CiphertextT v10) {
  const auto& v11 = v0->EvalMultNoRelin(v1, v2);
  EvalNoiseBGV(v0, secretKey, v11, "57.67", "v11=EvalMultNoRelin v1, v2");
  const auto& v12 = v0->Relinearize(v11);
  EvalNoiseBGV(v0, secretKey, v12, "57.77", "v12=Relinearize v11");
  const auto& v13 = v0->ModReduce(v12);
  EvalNoiseBGV(v0, secretKey, v13, "22.82", "v13=ModReduce v12");
  const auto& v14 = v0->EvalMultNoRelin(v3, v4);
  EvalNoiseBGV(v0, secretKey, v14, "57.67", "v14=EvalMultNoRelin v3, v4");
  const auto& v15 = v0->Relinearize(v14);
  EvalNoiseBGV(v0, secretKey, v15, "57.77", "v15=Relinearize v14");
  const auto& v16 = v0->EvalMultNoRelin(v15, v5);
  EvalNoiseBGV(v0, secretKey, v16, "88.66", "v16=EvalMultNoRelin v15, v5");
  const auto& v17 = v0->Relinearize(v16);
  EvalNoiseBGV(v0, secretKey, v17, "88.66", "v17=Relinearize v16");
  const auto& v18 = v0->ModReduce(v17);
  EvalNoiseBGV(v0, secretKey, v18, "43.66", "v18=ModReduce v17");
  const auto& v19 = v0->EvalMultNoRelin(v13, v18);
  EvalNoiseBGV(v0, secretKey, v19, "70.57", "v19=EvalMultNoRelin v13, v18");
  const auto& v20 = v0->Relinearize(v19);
  EvalNoiseBGV(v0, secretKey, v20, "70.57", "v20=Relinearize v19");
  const auto& v21 = v0->ModReduce(v20);
  EvalNoiseBGV(v0, secretKey, v21, "25.59", "v21=ModReduce v20");
  const auto& v22 = v0->EvalMultNoRelin(v6, v7);
  EvalNoiseBGV(v0, secretKey, v22, "57.67", "v22=EvalMultNoRelin v6, v7");
  const auto& v23 = v0->Relinearize(v22);
  EvalNoiseBGV(v0, secretKey, v23, "57.77", "v23=Relinearize v22");
  const auto& v24 = v0->ModReduce(v23);
  EvalNoiseBGV(v0, secretKey, v24, "22.82", "v24=ModReduce v23");
  const auto& v25 = v0->EvalMultNoRelin(v8, v9);
  EvalNoiseBGV(v0, secretKey, v25, "57.67", "v25=EvalMultNoRelin v8, v9");
  const auto& v26 = v0->Relinearize(v25);
  EvalNoiseBGV(v0, secretKey, v26, "57.77", "v26=Relinearize v25");
  const auto& v27 = v0->EvalMultNoRelin(v26, v10);
  EvalNoiseBGV(v0, secretKey, v27, "88.66", "v27=EvalMultNoRelin v26, v10");
  const auto& v28 = v0->Relinearize(v27);
  EvalNoiseBGV(v0, secretKey, v28, "88.66", "v28=Relinearize v27");
  const auto& v29 = v0->ModReduce(v28);
  EvalNoiseBGV(v0, secretKey, v29, "43.66", "v29=ModReduce v28");
  const auto& v30 = v0->EvalMultNoRelin(v24, v29);
  EvalNoiseBGV(v0, secretKey, v30, "70.57", "v30=EvalMultNoRelin v24, v29");
  const auto& v31 = v0->Relinearize(v30);
  EvalNoiseBGV(v0, secretKey, v31, "70.57", "v31=Relinearize v30");
  const auto& v32 = v0->ModReduce(v31);
  EvalNoiseBGV(v0, secretKey, v32, "25.59", "v32=ModReduce v31");
  const auto& v33 = v0->EvalMultNoRelin(v21, v32);
  EvalNoiseBGV(v0, secretKey, v33, "55.28", "v33=EvalMultNoRelin v21, v32");
  const auto& v34 = v0->Relinearize(v33);
  EvalNoiseBGV(v0, secretKey, v34, "56.08", "v34=Relinearize v33");
  const auto& v35 = v0->ModReduce(v34);
  EvalNoiseBGV(v0, secretKey, v35, "22.82", "v35=ModReduce v34");
  return v35;
}
CiphertextT func__encrypt__arg0(PrivateKeyT secretKey, CryptoContextT v36,
                                int16_t v37, PublicKeyT v38) {
  std::vector<int16_t> v39(8, v37);
  std::vector<int64_t> v40(std::begin(v39), std::end(v39));
  const auto& v41 = v36->MakePackedPlaintext(v40);
  const auto& v42 = v36->Encrypt(v38, v41);
  EvalNoiseBGV(v36, secretKey, v42, "26.5", "v42=Encrypt v38, v41");
  return v42;
}
CiphertextT func__encrypt__arg1(PrivateKeyT secretKey, CryptoContextT v43,
                                int16_t v44, PublicKeyT v45) {
  std::vector<int16_t> v46(8, v44);
  std::vector<int64_t> v47(std::begin(v46), std::end(v46));
  const auto& v48 = v43->MakePackedPlaintext(v47);
  const auto& v49 = v43->Encrypt(v45, v48);
  EvalNoiseBGV(v43, secretKey, v49, "26.5", "v49=Encrypt v45, v48");
  return v49;
}
CiphertextT func__encrypt__arg2(PrivateKeyT secretKey, CryptoContextT v50,
                                int16_t v51, PublicKeyT v52) {
  std::vector<int16_t> v53(8, v51);
  std::vector<int64_t> v54(std::begin(v53), std::end(v53));
  const auto& v55 = v50->MakePackedPlaintext(v54);
  const auto& v56 = v50->Encrypt(v52, v55);
  EvalNoiseBGV(v50, secretKey, v56, "26.5", "v56=Encrypt v52, v55");
  return v56;
}
CiphertextT func__encrypt__arg3(PrivateKeyT secretKey, CryptoContextT v57,
                                int16_t v58, PublicKeyT v59) {
  std::vector<int16_t> v60(8, v58);
  std::vector<int64_t> v61(std::begin(v60), std::end(v60));
  const auto& v62 = v57->MakePackedPlaintext(v61);
  const auto& v63 = v57->Encrypt(v59, v62);
  EvalNoiseBGV(v57, secretKey, v63, "26.5", "v63=Encrypt v59, v62");
  return v63;
}
CiphertextT func__encrypt__arg4(PrivateKeyT secretKey, CryptoContextT v64,
                                int16_t v65, PublicKeyT v66) {
  std::vector<int16_t> v67(8, v65);
  std::vector<int64_t> v68(std::begin(v67), std::end(v67));
  const auto& v69 = v64->MakePackedPlaintext(v68);
  const auto& v70 = v64->Encrypt(v66, v69);
  EvalNoiseBGV(v64, secretKey, v70, "26.5", "v70=Encrypt v66, v69");
  return v70;
}
CiphertextT func__encrypt__arg5(PrivateKeyT secretKey, CryptoContextT v71,
                                int16_t v72, PublicKeyT v73) {
  std::vector<int16_t> v74(8, v72);
  std::vector<int64_t> v75(std::begin(v74), std::end(v74));
  const auto& v76 = v71->MakePackedPlaintext(v75);
  const auto& v77 = v71->Encrypt(v73, v76);
  EvalNoiseBGV(v71, secretKey, v77, "26.5", "v77=Encrypt v73, v76");
  return v77;
}
CiphertextT func__encrypt__arg6(PrivateKeyT secretKey, CryptoContextT v78,
                                int16_t v79, PublicKeyT v80) {
  std::vector<int16_t> v81(8, v79);
  std::vector<int64_t> v82(std::begin(v81), std::end(v81));
  const auto& v83 = v78->MakePackedPlaintext(v82);
  const auto& v84 = v78->Encrypt(v80, v83);
  EvalNoiseBGV(v78, secretKey, v84, "26.5", "v84=Encrypt v80, v83");
  return v84;
}
CiphertextT func__encrypt__arg7(PrivateKeyT secretKey, CryptoContextT v85,
                                int16_t v86, PublicKeyT v87) {
  std::vector<int16_t> v88(8, v86);
  std::vector<int64_t> v89(std::begin(v88), std::end(v88));
  const auto& v90 = v85->MakePackedPlaintext(v89);
  const auto& v91 = v85->Encrypt(v87, v90);
  EvalNoiseBGV(v85, secretKey, v91, "26.5", "v91=Encrypt v87, v90");
  return v91;
}
CiphertextT func__encrypt__arg8(PrivateKeyT secretKey, CryptoContextT v92,
                                int16_t v93, PublicKeyT v94) {
  std::vector<int16_t> v95(8, v93);
  std::vector<int64_t> v96(std::begin(v95), std::end(v95));
  const auto& v97 = v92->MakePackedPlaintext(v96);
  const auto& v98 = v92->Encrypt(v94, v97);
  EvalNoiseBGV(v92, secretKey, v98, "26.5", "v98=Encrypt v94, v97");
  return v98;
}
CiphertextT func__encrypt__arg9(PrivateKeyT secretKey, CryptoContextT v99,
                                int16_t v100, PublicKeyT v101) {
  std::vector<int16_t> v102(8, v100);
  std::vector<int64_t> v103(std::begin(v102), std::end(v102));
  const auto& v104 = v99->MakePackedPlaintext(v103);
  const auto& v105 = v99->Encrypt(v101, v104);
  EvalNoiseBGV(v99, secretKey, v105, "26.5", "v105=Encrypt v101, v104");
  return v105;
}
int16_t func__decrypt__result0(PrivateKeyT secretKey, CryptoContextT v106,
                               CiphertextT v107, PrivateKeyT v108) {
  PlaintextT v109;
  v106->Decrypt(v108, v107, &v109);
  int16_t v110 = v109->GetPackedValue()[0];
  return v110;
}
CryptoContextT func__generate_crypto_context() {
  CCParamsT v111;
  v111.SetMultiplicativeDepth(3);
  v111.SetPlaintextModulus(65537);
  v111.SetSecurityLevel(HEStd_NotSet);
  v111.SetRingDim(8192);
  v111.SetMaxRelinSkDeg(2);
  v111.SetScalingTechnique(FIXEDMANUAL);
  v111.SetScalingModSize(45);
  v111.SetKeySwitchTechnique(BV);
  v111.SetDigitSize(30);
  v111.SetNumLargeDigits(0);
  CryptoContextT v112 = GenCryptoContext(v111);
  v112->Enable(PKE);
  v112->Enable(KEYSWITCH);
  v112->Enable(LEVELEDSHE);
  return v112;
}
CryptoContextT func__configure_crypto_context(PrivateKeyT secretKey,
                                              CryptoContextT v113,
                                              PrivateKeyT v114) {
  v113->EvalMultKeyGen(v114);
  return v113;
}
