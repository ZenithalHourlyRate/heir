
#include "src/pke/include/openfhe.h"  // from @openfhe

using namespace lbcrypto;
using CiphertextT = ConstCiphertext<DCRTPoly>;
using CCParamsT = CCParams<CryptoContextBGVRNS>;
using CryptoContextT = CryptoContext<DCRTPoly>;
using EvalKeyT = EvalKey<DCRTPoly>;
using PlaintextT = Plaintext;
using PrivateKeyT = PrivateKey<DCRTPoly>;
using PublicKeyT = PublicKey<DCRTPoly>;

CiphertextT func(CryptoContextT v0, CiphertextT v1, CiphertextT v2);
CiphertextT func__encrypt__arg0(CryptoContextT v16, std::vector<int16_t> v17,
                                PublicKeyT v18);
CiphertextT func__encrypt__arg1(CryptoContextT v22, std::vector<int16_t> v23,
                                PublicKeyT v24);
int16_t func__decrypt__result0(CryptoContextT v28, CiphertextT v29,
                               PrivateKeyT v30);
CryptoContextT func__generate_crypto_context();
CryptoContextT func__configure_crypto_context(CryptoContextT v35,
                                              PrivateKeyT v36);
