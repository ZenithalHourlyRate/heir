#include <cstdint>
#include <vector>

#include "func.h"
#include "src/pke/include/openfhe.h"  // from @openfhe

int main(int argc, char *argv[]) {
  CryptoContext<DCRTPoly> cryptoContext = func__generate_crypto_context();
  KeyPair<DCRTPoly> keyPair;
  keyPair = cryptoContext->KeyGen();
  cryptoContext =
      func__configure_crypto_context(cryptoContext, keyPair.secretKey);

  std::cout << *(cryptoContext->GetCryptoParameters()) << std::endl;

  int32_t n = cryptoContext->GetCryptoParameters()
                  ->GetElementParams()
                  ->GetCyclotomicOrder() /
              2;
  int16_t arg0Vals[8] = {1, 2, 3, 4, 5, 6, 7, 8};
  int16_t arg1Vals[8] = {2, 3, 4, 5, 6, 7, 8, 9};
  int64_t expected = 240;

  std::vector<int16_t> arg0;
  std::vector<int16_t> arg1;
  arg0.reserve(n);
  arg1.reserve(n);

  // TODO(#645): support cyclic repetition in add-client-interface
  for (int i = 0; i < n; ++i) {
    arg0.push_back(arg0Vals[i % 8]);
    arg1.push_back(arg1Vals[i % 8]);
  }

  auto arg0Encrypted =
      func__encrypt__arg0(cryptoContext, arg0, keyPair.publicKey);
  auto arg1Encrypted =
      func__encrypt__arg1(cryptoContext, arg1, keyPair.publicKey);
  auto outputEncrypted = func(cryptoContext, arg0Encrypted, arg1Encrypted);
  auto actual =
      func__decrypt__result0(cryptoContext, outputEncrypted, keyPair.secretKey);

  std::cout << "Expected: " << expected << "\n";
  std::cout << "Actual: " << actual << "\n";

  return 0;
}
