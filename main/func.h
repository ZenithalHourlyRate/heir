
#include "src/pke/include/openfhe.h"  // from @openfhe

using namespace lbcrypto;
using CiphertextT = ConstCiphertext<DCRTPoly>;
using CCParamsT = CCParams<CryptoContextBGVRNS>;
using CryptoContextT = CryptoContext<DCRTPoly>;
using EvalKeyT = EvalKey<DCRTPoly>;
using PlaintextT = Plaintext;
using PrivateKeyT = PrivateKey<DCRTPoly>;
using PublicKeyT = PublicKey<DCRTPoly>;

CiphertextT func(PrivateKeyT secretKey, CryptoContextT v0, CiphertextT v1,
                 CiphertextT v2, CiphertextT v3, CiphertextT v4, CiphertextT v5,
                 CiphertextT v6, CiphertextT v7, CiphertextT v8);
CiphertextT func__encrypt__arg0(PrivateKeyT secretKey, CryptoContextT v28,
                                int16_t v29, PublicKeyT v30);
CiphertextT func__encrypt__arg1(PrivateKeyT secretKey, CryptoContextT v35,
                                int16_t v36, PublicKeyT v37);
CiphertextT func__encrypt__arg2(PrivateKeyT secretKey, CryptoContextT v42,
                                int16_t v43, PublicKeyT v44);
CiphertextT func__encrypt__arg3(PrivateKeyT secretKey, CryptoContextT v49,
                                int16_t v50, PublicKeyT v51);
CiphertextT func__encrypt__arg4(PrivateKeyT secretKey, CryptoContextT v56,
                                int16_t v57, PublicKeyT v58);
CiphertextT func__encrypt__arg5(PrivateKeyT secretKey, CryptoContextT v63,
                                int16_t v64, PublicKeyT v65);
CiphertextT func__encrypt__arg6(PrivateKeyT secretKey, CryptoContextT v70,
                                int16_t v71, PublicKeyT v72);
CiphertextT func__encrypt__arg7(PrivateKeyT secretKey, CryptoContextT v77,
                                int16_t v78, PublicKeyT v79);
int16_t func__decrypt__result0(PrivateKeyT secretKey, CryptoContextT v84,
                               CiphertextT v85, PrivateKeyT v86);
CryptoContextT func__generate_crypto_context();
CryptoContextT func__configure_crypto_context(PrivateKeyT secretKey,
                                              CryptoContextT v91,
                                              PrivateKeyT v92);
