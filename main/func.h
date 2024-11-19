
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
CiphertextT func__encrypt__arg0(PrivateKeyT secretKey, CryptoContextT v12,
                                int16_t v13, PublicKeyT v14);
CiphertextT func__encrypt__arg1(PrivateKeyT secretKey, CryptoContextT v19,
                                int16_t v20, PublicKeyT v21);
CiphertextT func__encrypt__arg2(PrivateKeyT secretKey, CryptoContextT v26,
                                int16_t v27, PublicKeyT v28);
CiphertextT func__encrypt__arg3(PrivateKeyT secretKey, CryptoContextT v33,
                                int16_t v34, PublicKeyT v35);
CiphertextT func__encrypt__arg4(PrivateKeyT secretKey, CryptoContextT v40,
                                int16_t v41, PublicKeyT v42);
CiphertextT func__encrypt__arg5(PrivateKeyT secretKey, CryptoContextT v47,
                                int16_t v48, PublicKeyT v49);
CiphertextT func__encrypt__arg6(PrivateKeyT secretKey, CryptoContextT v54,
                                int16_t v55, PublicKeyT v56);
CiphertextT func__encrypt__arg7(PrivateKeyT secretKey, CryptoContextT v61,
                                int16_t v62, PublicKeyT v63);
int16_t func__decrypt__result0(PrivateKeyT secretKey, CryptoContextT v68,
                               CiphertextT v69, PrivateKeyT v70);
CryptoContextT func__generate_crypto_context();
CryptoContextT func__configure_crypto_context(PrivateKeyT secretKey,
                                              CryptoContextT v75,
                                              PrivateKeyT v76);
