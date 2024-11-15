
#include "src/pke/include/openfhe.h"  // from @openfhe

using namespace lbcrypto;
using CiphertextT = ConstCiphertext<DCRTPoly>;
using CCParamsT = CCParams<CryptoContextBGVRNS>;
using CryptoContextT = CryptoContext<DCRTPoly>;
using EvalKeyT = EvalKey<DCRTPoly>;
using PlaintextT = Plaintext;
using PrivateKeyT = PrivateKey<DCRTPoly>;
using PublicKeyT = PublicKey<DCRTPoly>;

CiphertextT func(CryptoContextT v0, CiphertextT v1, CiphertextT v2,
                 CiphertextT v3, CiphertextT v4, CiphertextT v5, CiphertextT v6,
                 CiphertextT v7, CiphertextT v8);
CiphertextT func__encrypt__arg0(CryptoContextT v30, int16_t v31,
                                PublicKeyT v32);
CiphertextT func__encrypt__arg1(CryptoContextT v37, int16_t v38,
                                PublicKeyT v39);
CiphertextT func__encrypt__arg2(CryptoContextT v44, int16_t v45,
                                PublicKeyT v46);
CiphertextT func__encrypt__arg3(CryptoContextT v51, int16_t v52,
                                PublicKeyT v53);
CiphertextT func__encrypt__arg4(CryptoContextT v58, int16_t v59,
                                PublicKeyT v60);
CiphertextT func__encrypt__arg5(CryptoContextT v65, int16_t v66,
                                PublicKeyT v67);
CiphertextT func__encrypt__arg6(CryptoContextT v72, int16_t v73,
                                PublicKeyT v74);
CiphertextT func__encrypt__arg7(CryptoContextT v79, int16_t v80,
                                PublicKeyT v81);
int16_t func__decrypt__result0(CryptoContextT v86, CiphertextT v87,
                               PrivateKeyT v88);
CryptoContextT func__generate_crypto_context();
CryptoContextT func__configure_crypto_context(CryptoContextT v93,
                                              PrivateKeyT v94);
