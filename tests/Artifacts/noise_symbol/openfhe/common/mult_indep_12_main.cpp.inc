int main() {
  auto cryptoContext = mult_indep__generate_crypto_context();
  std::cout << *(cryptoContext->GetCryptoParameters()) << std::endl;
  auto keyPair = cryptoContext->KeyGen();
  auto publicKey = keyPair.publicKey;
  auto secretKey = keyPair.secretKey;
  cryptoContext =
      mult_indep__configure_crypto_context(cryptoContext, secretKey);

  int16_t arg0 = 1;
  int64_t expected = 1;

  auto arg0Encrypted =
      mult_indep__encrypt__arg0(cryptoContext, arg0, publicKey);
  auto arg1Encrypted =
      mult_indep__encrypt__arg1(cryptoContext, arg0, publicKey);
  auto arg2Encrypted =
      mult_indep__encrypt__arg2(cryptoContext, arg0, publicKey);
  auto arg3Encrypted =
      mult_indep__encrypt__arg3(cryptoContext, arg0, publicKey);
  auto arg4Encrypted =
      mult_indep__encrypt__arg4(cryptoContext, arg0, publicKey);
  auto arg5Encrypted =
      mult_indep__encrypt__arg5(cryptoContext, arg0, publicKey);
  auto arg6Encrypted =
      mult_indep__encrypt__arg6(cryptoContext, arg0, publicKey);
  auto arg7Encrypted =
      mult_indep__encrypt__arg7(cryptoContext, arg0, publicKey);
  auto arg8Encrypted =
      mult_indep__encrypt__arg8(cryptoContext, arg0, publicKey);
  auto arg9Encrypted =
      mult_indep__encrypt__arg9(cryptoContext, arg0, publicKey);
  auto arg10Encrypted =
      mult_indep__encrypt__arg10(cryptoContext, arg0, publicKey);
  auto arg11Encrypted =
      mult_indep__encrypt__arg11(cryptoContext, arg0, publicKey);
  auto outputEncrypted = mult_indep(
      cryptoContext, secretKey, arg0Encrypted, arg1Encrypted, arg2Encrypted,
      arg3Encrypted, arg4Encrypted, arg5Encrypted, arg6Encrypted, arg7Encrypted,
      arg8Encrypted, arg9Encrypted, arg10Encrypted, arg11Encrypted);
  auto actual =
      mult_indep__decrypt__result0(cryptoContext, outputEncrypted, secretKey);

  if (expected != actual) {
    std::cerr << "Test failed: expected " << expected << ", got " << actual
              << std::endl;
    return 1;
  }
  std::cout << "Test passed" << std::endl;
  return 0;
}
