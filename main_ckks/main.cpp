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

int main(int argc, char* argv[]) {
  CryptoContext<DCRTPoly> cryptoContext = func__generate_crypto_context();
  KeyPair<DCRTPoly> keyPair;
  keyPair = cryptoContext->KeyGen();
  cryptoContext = func__configure_crypto_context(
      keyPair.secretKey, cryptoContext, keyPair.secretKey);

  std::cout << *(cryptoContext->GetCryptoParameters()) << std::endl;

  std::vector<double> x1 = {1, 2, 3, 4, 1, 2, 3, 4};

  int32_t n = cryptoContext->GetCryptoParameters()
                  ->GetElementParams()
                  ->GetRingDimension() /
              2;
  std::vector<double> outputs;
  outputs.reserve(n);
  for (int i = 0; i < n; ++i) {
    outputs.push_back(x1[i % 8]);
  }
  const auto& ptxt1 = cryptoContext->MakeCKKSPackedPlaintext(outputs);
  const auto& c1 = cryptoContext->Encrypt(keyPair.publicKey, ptxt1);

  // std::vector<double> x1 = {0.25, 0.5, 0.75, 1.0, 2.0, 3.0, 4.0, 5.0};
  // std::vector<double> x2 = {5.0, 4.0, 3.0, 2.0, 1.0, 0.75, 0.5, 0.25};

  // Plaintext ptxt1 = cryptoContext->MakeCKKSPackedPlaintext(x1);
  // Plaintext ptxt2 = cryptoContext->MakeCKKSPackedPlaintext(x1);

  // auto c1 = cryptoContext->Encrypt(keyPair.publicKey, ptxt1);
  // auto c2 = cryptoContext->Encrypt(keyPair.publicKey, ptxt2);

  auto outputEncrypted = func(keyPair.secretKey, cryptoContext, c1);

  Plaintext result;
  cryptoContext->Decrypt(keyPair.secretKey, outputEncrypted, &result);

  result->SetLength(1);

  std::cout << "Expected: " << 60 << "\n";
  std::cout << "Actual: " << result << "\n";

  return 0;
}
