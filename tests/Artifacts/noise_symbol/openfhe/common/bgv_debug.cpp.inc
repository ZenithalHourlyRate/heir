// DecryptCore not accessible from CryptoContext
// so copy from @openfhe//src/pke/lib/schemerns/rns-pke.cpp
DCRTPoly DecryptCore(const std::vector<DCRTPoly>& cv,
                     const PrivateKey<DCRTPoly> privateKey) {
  const DCRTPoly& s = privateKey->GetPrivateElement();

  size_t sizeQ = s.GetParams()->GetParams().size();
  size_t sizeQl = cv[0].GetParams()->GetParams().size();

  size_t diffQl = sizeQ - sizeQl;

  auto scopy(s);
  scopy.DropLastElements(diffQl);

  DCRTPoly sPower(scopy);

  DCRTPoly b(cv[0]);
  b.SetFormat(Format::EVALUATION);

  DCRTPoly ci;
  for (size_t i = 1; i < cv.size(); i++) {
    ci = cv[i];
    ci.SetFormat(Format::EVALUATION);

    b += sPower * ci;
    sPower *= scopy;
  }
  return b;
}

#define OP
//#define DECRYPT
#define NOISE

void __heir_debug(CryptoContextT cc, PrivateKeyT sk, CiphertextT ct,
                  const std::map<std::string, std::string>& debugAttrMap) {
#ifdef OP
  auto isBlockArgument = debugAttrMap.at("asm.is_block_arg");
  if (isBlockArgument == "1") {
    // std::cout << "Input" << std::endl;
  } else {
    // std::cout << debugAttrMap.at("asm.op_name") << std::endl;
    if (debugAttrMap.at("asm.op_name") == "openfhe.relin") {
      // skip relin op
      return;
    }
  }
#endif

#ifdef DECRYPT
  PlaintextT ptxt;
  cc->Decrypt(sk, ct, &ptxt);
  ptxt->SetLength(std::stod(debugAttrMap.at("message.size")));
  std::cout << "  " << ptxt << std::endl;
#endif

#ifdef NOISE
  auto cv = ct->GetElements();
  size_t sizeQl = cv[0].GetParams()->GetParams().size();

  auto b = DecryptCore(cv, sk);
  b.SetFormat(Format::COEFFICIENT);

  double noise = (log2(b.Norm()));

  double logQ = 0;
  std::vector<double> logqi_v;
  for (usint i = 0; i < sizeQl; i++) {
    double logqi =
        log2(cv[0].GetParams()->GetParams()[i]->GetModulus().ConvertToInt());
    logqi_v.push_back(logqi);
    logQ += logqi;
  }

  // std::cout << "  cv " << cv.size() << " Ql " << sizeQl << " logQ: " << logQ
  //           << " logqi: " << logqi_v << " budget " << logQ - noise - 1
  //           << " noise: " << noise << std::endl;
  static int count = 0;
  std::cout << " count: " << count++ << std::endl;
  std::cout << " noise: " << noise << std::endl;

  // print the predicted bound by analysis
  if (debugAttrMap.find("noise.bound") != debugAttrMap.end()) {
    double noiseBound = std::stod(debugAttrMap.at("noise.bound"));

    std::cout << " noise bound: " << noiseBound
              << " gap: " << noiseBound - noise << std::endl;
  }
#endif
}
