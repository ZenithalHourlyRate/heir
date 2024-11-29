#include <cstdint>
#include <vector>

#include "func.h"
#include "src/pke/include/openfhe.h"  // from @openfhe

void EvalNoiseCKKS(CryptoContext<DCRTPoly> cryptoContext,
                   PrivateKey<DCRTPoly> privateKey,
                   ConstCiphertext<DCRTPoly> ciphertext, std::string bound,
                   std::string tag) {
  Plaintext ptxt;
  cryptoContext->Decrypt(privateKey, ciphertext, &ptxt);
  ptxt->SetLength(8);

  const std::vector<DCRTPoly>& cv = ciphertext->GetElements();

  size_t sizeQl = cv[0].GetParams()->GetParams().size();

  auto noise = ptxt->GetLogError();

  std::cout << tag << std::endl;
  // << '\t' << "cv " << cv.size() << " Ql "
  // << sizeQl
  // << " noise: " << noise
  // << " bound " << bound << std::endl;

  std::cout << "result decrypted: " << ptxt << std::endl;
}

CiphertextT func__encrypt__arg0(CryptoContextT v16, std::vector<double> v17,
                                PublicKeyT v18) {
  int32_t n =
      v16->GetCryptoParameters()->GetElementParams()->GetRingDimension() / 2;
  std::vector<double> outputs;
  outputs.reserve(n);
  for (int i = 0; i < n; ++i) {
    outputs.push_back(v17[i % v17.size()]);
  }
  const auto& v19 = v16->MakeCKKSPackedPlaintext(outputs);
  const auto& v20 = v16->Encrypt(v18, v19);
  return v20;
}

double func__decrypt__result0(CryptoContextT v26, CiphertextT v27,
                              PrivateKeyT v28) {
  PlaintextT v29;
  v26->Decrypt(v28, v27, &v29);
  double v30 = v29->GetCKKSPackedValue()[0].real();
  return v30;
}

int main(int argc, char* argv[]) {
  CryptoContext<DCRTPoly> cryptoContext = func__generate_crypto_context();
  KeyPair<DCRTPoly> keyPair;
  keyPair = cryptoContext->KeyGen();
  cryptoContext = func__configure_crypto_context(
      keyPair.secretKey, cryptoContext, keyPair.secretKey);

  std::cout << *(cryptoContext->GetCryptoParameters()) << std::endl;

  std::vector<double> arg0 = {0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8};
  std::vector<double> arg1 = {0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9};
  double expected = 2.4 + 0.1;

  auto arg0Encrypted =
      func__encrypt__arg0(cryptoContext, arg0, keyPair.publicKey);
  auto arg1Encrypted =
      func__encrypt__arg0(cryptoContext, arg1, keyPair.publicKey);
  auto outputEncrypted =
      func(keyPair.secretKey, cryptoContext, arg0Encrypted, arg1Encrypted);
  auto actual =
      func__decrypt__result0(cryptoContext, outputEncrypted, keyPair.secretKey);

  std::cout << "Expected: " << expected << "\n";
  std::cout << "Actual: " << actual << "\n";

  return 0;
}
