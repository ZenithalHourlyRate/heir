#include <cstdint>
#include <vector>

#include "func.h"
#include "src/pke/include/openfhe.h"  // from @openfhe

void EvalNoiseBGV(CryptoContext<DCRTPoly> cryptoContext,
                  PrivateKey<DCRTPoly> privateKey,
                  ConstCiphertext<DCRTPoly> ciphertext, std::string bound,
                  std::string tag) {
  Plaintext ptxt;
  cryptoContext->Decrypt(privateKey, ciphertext, &ptxt);
  ptxt->SetLength(8);
  // std::cout << '\n' << tag << '\t' << "decrypted: " << ptxt << std::endl;
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

  std::cout << tag << '\t' << "cv " << cv.size() << " Ql "
            << sizeQl
            // << " logQ: " << logQ << " logqi: " << logqi_v
            << " budget " << logQ - noise - 1 << " noise: " << noise
            << " bound " << bound << " gap "
            << stod(bound.substr(0, bound.find(' '))) - noise << std::endl;
}

int main(int argc, char* argv[]) {
  CryptoContext<DCRTPoly> cryptoContext = func__generate_crypto_context();
  KeyPair<DCRTPoly> keyPair;
  keyPair = cryptoContext->KeyGen();
  cryptoContext = func__configure_crypto_context(
      keyPair.secretKey, cryptoContext, keyPair.secretKey);

  std::cout << *(cryptoContext->GetCryptoParameters()) << std::endl;

#define DOTPRODUCT
  // #define MULT8

#ifdef DOTPRODUCT
  std::vector<int16_t> arg0 = {1, 2, 3, 4, 5, 6, 7, 8};
  std::vector<int16_t> arg1 = {2, 3, 4, 5, 6, 7, 8, 9};
  int64_t expected = 240;

  auto arg0Encrypted = func__encrypt__arg0(keyPair.secretKey, cryptoContext,
                                           arg0, keyPair.publicKey);
  auto arg1Encrypted = func__encrypt__arg1(keyPair.secretKey, cryptoContext,
                                           arg1, keyPair.publicKey);
#endif
#ifdef MULT8
  auto arg0Encrypted = func__encrypt__arg0(keyPair.secretKey, cryptoContext, 0,
                                           keyPair.publicKey);
  auto arg1Encrypted = func__encrypt__arg1(keyPair.secretKey, cryptoContext, 1,
                                           keyPair.publicKey);
  auto arg2Encrypted = func__encrypt__arg2(keyPair.secretKey, cryptoContext, 0,
                                           keyPair.publicKey);
  auto arg3Encrypted = func__encrypt__arg3(keyPair.secretKey, cryptoContext, 1,
                                           keyPair.publicKey);
  auto arg4Encrypted = func__encrypt__arg4(keyPair.secretKey, cryptoContext, 0,
                                           keyPair.publicKey);
  auto arg5Encrypted = func__encrypt__arg5(keyPair.secretKey, cryptoContext, 1,
                                           keyPair.publicKey);
  auto arg6Encrypted = func__encrypt__arg6(keyPair.secretKey, cryptoContext, 0,
                                           keyPair.publicKey);
  auto arg7Encrypted = func__encrypt__arg7(keyPair.secretKey, cryptoContext, 1,
                                           keyPair.publicKey);
#endif

#ifdef MULT8
  auto outputEncrypted =
      func(keyPair.secretKey, cryptoContext, arg0Encrypted, arg1Encrypted,
           arg2Encrypted, arg3Encrypted, arg4Encrypted, arg5Encrypted,
           arg6Encrypted, arg7Encrypted);
#endif
#ifdef DOTPRODUCT
  auto outputEncrypted =
      func(keyPair.secretKey, cryptoContext, arg0Encrypted, arg1Encrypted);
#endif
  auto actual = func__decrypt__result0(keyPair.secretKey, cryptoContext,
                                       outputEncrypted, keyPair.secretKey);

  // std::cout << "Expected: " << expected << "\n";
  // std::cout << "Actual: " << actual << "\n";

  return 0;
}
