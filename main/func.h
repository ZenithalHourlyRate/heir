
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
CiphertextT func__encrypt__arg0(PrivateKeyT secretKey, CryptoContextT v31,
                                int16_t v32, PublicKeyT v33);
CiphertextT func__encrypt__arg1(PrivateKeyT secretKey, CryptoContextT v38,
                                int16_t v39, PublicKeyT v40);
CiphertextT func__encrypt__arg2(PrivateKeyT secretKey, CryptoContextT v45,
                                int16_t v46, PublicKeyT v47);
CiphertextT func__encrypt__arg3(PrivateKeyT secretKey, CryptoContextT v52,
                                int16_t v53, PublicKeyT v54);
CiphertextT func__encrypt__arg4(PrivateKeyT secretKey, CryptoContextT v59,
                                int16_t v60, PublicKeyT v61);
CiphertextT func__encrypt__arg5(PrivateKeyT secretKey, CryptoContextT v66,
                                int16_t v67, PublicKeyT v68);
CiphertextT func__encrypt__arg6(PrivateKeyT secretKey, CryptoContextT v73,
                                int16_t v74, PublicKeyT v75);
CiphertextT func__encrypt__arg7(PrivateKeyT secretKey, CryptoContextT v80,
                                int16_t v81, PublicKeyT v82);
int16_t func__decrypt__result0(PrivateKeyT secretKey, CryptoContextT v87,
                               CiphertextT v88, PrivateKeyT v89);
CryptoContextT func__generate_crypto_context();
CryptoContextT func__configure_crypto_context(PrivateKeyT secretKey,
                                              CryptoContextT v94,
                                              PrivateKeyT v95);
