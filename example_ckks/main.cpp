#include <cstdint>
#include <vector>

#include "func.h"
#include "src/pke/include/openfhe.h"  // from @openfhe

using namespace lbcrypto;

int main(int argc, char* argv[]) {
  CryptoContext<DCRTPoly> cryptoContext = func__generate_crypto_context();
  KeyPair<DCRTPoly> keyPair;
  keyPair = cryptoContext->KeyGen();
  cryptoContext =
      func__configure_crypto_context(cryptoContext, keyPair.secretKey);

  uint32_t levelsAvailableAfterBootstrap = 10;

  std::cout << *(cryptoContext->GetCryptoParameters()) << std::endl;

  auto depth = levelsAvailableAfterBootstrap + 3 + 14 + 3;

  std::cout << "depth: " << depth << std::endl;

  std::vector<double> x = {0.25, 0.5, 0.75, 1.0, 2.0, 3.0, 4.0, 5.0};
  size_t encodedLength = x.size();

  // We start with a depleted ciphertext that has used up all of its levels.
  Plaintext ptxt = cryptoContext->MakeCKKSPackedPlaintext(x, 1, depth - 1);

  ptxt->SetLength(encodedLength);
  std::cout << "Input: " << ptxt << std::endl;

  Ciphertext<DCRTPoly> encrypted =
      cryptoContext->Encrypt(keyPair.publicKey, ptxt);

  std::cout << "Initial number of levels remaining: "
            << depth - encrypted->GetLevel() << std::endl;

  auto out = func(cryptoContext, encrypted);

  std::cout << "Number of levels remaining after bootstrapping: "
            << depth - out->GetLevel() - (out->GetNoiseScaleDeg() - 1)
            << std::endl
            << std::endl;

  Plaintext result;
  cryptoContext->Decrypt(keyPair.secretKey, out, &result);
  result->SetLength(encodedLength);
  std::cout << "Output after bootstrapping \n\t" << result << std::endl;

  return 0;
}
