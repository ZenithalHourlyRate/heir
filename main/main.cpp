#include <cstdint>
#include <vector>

#include "func.h"
#include "src/pke/include/openfhe.h"  // from @openfhe

void EvalNoiseBGV(CryptoContext<DCRTPoly> cryptoContext,
                  PrivateKey<DCRTPoly> privateKey,
                  ConstCiphertext<DCRTPoly> ciphertext, std::string tag) {
  Plaintext ptxt;
  cryptoContext->Decrypt(privateKey, ciphertext, &ptxt);
  ptxt->SetLength(8);
  std::cout << '\n' << tag << '\t' << "decrypted: " << ptxt << std::endl;
  // const auto ptm =
  // cryptoContext->GetCryptoParameters()->GetPlaintextModulus();

  const std::vector<DCRTPoly>& cv = ciphertext->GetElements();
  DCRTPoly s = privateKey->GetPrivateElement();

  size_t sizeQl = cv[0].GetParams()->GetParams().size();
  size_t sizeQs = s.GetParams()->GetParams().size();

  size_t diffQl = sizeQs - sizeQl;

  auto scopy(s);
  scopy.DropLastElements(diffQl);

  DCRTPoly sPower(scopy);

  DCRTPoly b = cv[0];
  b.SetFormat(Format::EVALUATION);

  DCRTPoly ci;
  for (size_t i = 1; i < cv.size(); i++) {
    ci = cv[i];
    ci.SetFormat(Format::EVALUATION);

    b += sPower * ci;
    sPower *= scopy;
  }

  b.SetFormat(Format::COEFFICIENT);
  Poly b_big = b.CRTInterpolate();

  Poly plain_big;

  DCRTPoly plain_dcrt = ptxt->GetElement<DCRTPoly>();
  auto plain_dcrt_size = plain_dcrt.GetNumOfElements();

  if (plain_dcrt_size > 0) {
    plain_dcrt.SetFormat(Format::COEFFICIENT);
    plain_big = plain_dcrt.CRTInterpolate();
  } else {
    std::vector<int64_t> value = ptxt->GetPackedValue();
    Plaintext repack = cryptoContext->MakePackedPlaintext(value);
    DCRTPoly plain_repack = repack->GetElement<DCRTPoly>();
    plain_repack.SetFormat(Format::COEFFICIENT);
    plain_big = plain_repack.CRTInterpolate();
  }

  auto plain_modulus = plain_big.GetModulus();
  auto b_modulus = b_big.GetModulus();
  plain_big.SwitchModulus(b_big.GetModulus(), b_big.GetRootOfUnity(), 0, 0);

  Poly res = b_big - plain_big;

  double noise = (log2(res.Norm()));

  double logQ = 0;
  std::vector<double> logqi_v;
  for (usint i = 0; i < sizeQl; i++) {
    double logqi =
        log2(cv[0].GetParams()->GetParams()[i]->GetModulus().ConvertToInt());
    logqi_v.push_back(logqi);
    logQ += logqi;
  }

  std::cout << tag << '\t' << "cv " << cv.size() << " Ql " << sizeQl
            << " logQ: " << logQ << " logqi: " << logqi_v << " noise: " << noise
            << " budget " << logQ - noise - 1 << std::endl;
}

int main(int argc, char* argv[]) {
  CryptoContext<DCRTPoly> cryptoContext = func__generate_crypto_context();
  KeyPair<DCRTPoly> keyPair;
  keyPair = cryptoContext->KeyGen();
  cryptoContext = func__configure_crypto_context(
      keyPair.secretKey, cryptoContext, keyPair.secretKey);

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

  auto arg0Encrypted = func__encrypt__arg0(keyPair.secretKey, cryptoContext,
                                           arg0, keyPair.publicKey);
  auto arg1Encrypted = func__encrypt__arg1(keyPair.secretKey, cryptoContext,
                                           arg1, keyPair.publicKey);
  // auto arg0Encrypted = func__encrypt__arg0(keyPair.secretKey, cryptoContext,
  //                                          1, keyPair.publicKey);
  // auto arg1Encrypted = func__encrypt__arg1(keyPair.secretKey, cryptoContext,
  //                                          2, keyPair.publicKey);
  auto outputEncrypted =
      func(keyPair.secretKey, cryptoContext, arg0Encrypted, arg1Encrypted);
  auto actual = func__decrypt__result0(keyPair.secretKey, cryptoContext,
                                       outputEncrypted, keyPair.secretKey);

  // std::cout << "Expected: " << expected << "\n";
  std::cout << "Actual: " << actual << "\n";

  return 0;
}
