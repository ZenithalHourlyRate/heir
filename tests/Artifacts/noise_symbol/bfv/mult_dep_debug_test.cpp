#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <map>
#include <ostream>
#include <string>
#include <vector>

#include "openfhe.h"

// Generated headers (block clang-format from messing up order)
#include "mult_dep_lib.h"

// Debug handler (block clang-format from messing up order)
#include "bfv_debug.cpp.inc"

int main() {
  auto cryptoContext = mult_dep__generate_crypto_context();
  std::cout << *(cryptoContext->GetCryptoParameters()) << std::endl;
  auto keyPair = cryptoContext->KeyGen();
  auto publicKey = keyPair.publicKey;
  auto secretKey = keyPair.secretKey;
  cryptoContext = mult_dep__configure_crypto_context(cryptoContext, secretKey);

  int16_t arg0 = 1;
  int64_t expected = 1;

  auto arg0Encrypted = mult_dep__encrypt__arg0(cryptoContext, arg0, publicKey);
  auto outputEncrypted = mult_dep(cryptoContext, secretKey, arg0Encrypted);
  auto actual =
      mult_dep__decrypt__result0(cryptoContext, outputEncrypted, secretKey);

  if (expected != actual) {
    std::cerr << "Test failed: expected " << expected << ", got " << actual
              << std::endl;
    return 1;
  }
  std::cout << "Test passed" << std::endl;
  return 0;
}
