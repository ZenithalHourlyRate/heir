
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
CiphertextT func__encrypt__arg0(PrivateKeyT secretKey, CryptoContextT v22,
                                int16_t v23, PublicKeyT v24);
CiphertextT func__encrypt__arg1(PrivateKeyT secretKey, CryptoContextT v29,
                                int16_t v30, PublicKeyT v31);
CiphertextT func__encrypt__arg2(PrivateKeyT secretKey, CryptoContextT v36,
                                int16_t v37, PublicKeyT v38);
CiphertextT func__encrypt__arg3(PrivateKeyT secretKey, CryptoContextT v43,
                                int16_t v44, PublicKeyT v45);
CiphertextT func__encrypt__arg4(PrivateKeyT secretKey, CryptoContextT v50,
                                int16_t v51, PublicKeyT v52);
CiphertextT func__encrypt__arg5(PrivateKeyT secretKey, CryptoContextT v57,
                                int16_t v58, PublicKeyT v59);
CiphertextT func__encrypt__arg6(PrivateKeyT secretKey, CryptoContextT v64,
                                int16_t v65, PublicKeyT v66);
CiphertextT func__encrypt__arg7(PrivateKeyT secretKey, CryptoContextT v71,
                                int16_t v72, PublicKeyT v73);
int16_t func__decrypt__result0(PrivateKeyT secretKey, CryptoContextT v78,
                               CiphertextT v79, PrivateKeyT v80);
CryptoContextT func__generate_crypto_context();
CryptoContextT func__configure_crypto_context(PrivateKeyT secretKey,
                                              CryptoContextT v85,
                                              PrivateKeyT v86);
