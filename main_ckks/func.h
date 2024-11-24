
#include "src/pke/include/openfhe.h"  // from @openfhe

using namespace lbcrypto;
using CiphertextT = ConstCiphertext<DCRTPoly>;
using CCParamsT = CCParams<CryptoContextCKKSRNS>;
using CryptoContextT = CryptoContext<DCRTPoly>;
using EvalKeyT = EvalKey<DCRTPoly>;
using PlaintextT = Plaintext;
using PrivateKeyT = PrivateKey<DCRTPoly>;
using PublicKeyT = PublicKey<DCRTPoly>;

CiphertextT func(PrivateKeyT secretKey, CryptoContextT v0, CiphertextT v1);
CryptoContextT func__generate_crypto_context();
CryptoContextT func__configure_crypto_context(PrivateKeyT secretKey,
                                              CryptoContextT v10,
                                              PrivateKeyT v11);
