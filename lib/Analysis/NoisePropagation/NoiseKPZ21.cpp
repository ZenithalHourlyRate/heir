#include "lib/Analysis/NoisePropagation/NoiseKPZ21.h"

#include <cmath>

#include "lib/Analysis/NoisePropagation/Params.h"
#include "llvm/include/llvm/Support/Debug.h"        // from @llvm-project
#include "llvm/include/llvm/Support/raw_ostream.h"  // from @llvm-project

#define DEBUG_TYPE "NoiseKPZ"

namespace mlir {
namespace heir {

std::string NoiseKPZ::toString() const {
  switch (varianceType) {
    case (NoiseKPZType::UNINITIALIZED):
      return "NoiseKPZ(uninitialized)";
    case (NoiseKPZType::UNBOUNDED):
      return "NoiseKPZ(unbounded)";
    case (NoiseKPZType::SET):
      return "NoiseKPZ(" + std::to_string(log(getValue()) / log(2)) + ") ";
  }
}

std::string NoiseKPZ::toBound(const LocalParam &param) const {
  if (varianceType == NoiseKPZType::UNBOUNDED) return "MAX";
  auto t = param.getSchemeParam()->t;
  auto bound = log(t * getValue()) / log(2);
  std::stringstream stream;
  stream << std::fixed << std::setprecision(2) << bound;
  return stream.str();
}

llvm::raw_ostream &operator<<(llvm::raw_ostream &os, const NoiseKPZ &variance) {
  return os << variance.toString();
}

Diagnostic &operator<<(Diagnostic &diagnostic, const NoiseKPZ &variance) {
  return diagnostic << variance.toString();
}

double NoiseKPZ::getExpansionFactor(const LocalParam &param) {
  auto n = param.getSchemeParam()->n;
  // openfhe average case
  auto expansionFactor = 2.0 * sqrt(n);
  // worst-case
  // auto expansionFactor = n;
  return expansionFactor;
}

double NoiseKPZ::getBoundErr(const LocalParam &param) {
  auto std0 = param.getSchemeParam()->std0;
  auto assurance = 6;
  auto boundErr = std0 * assurance;
  return boundErr;
}

double NoiseKPZ::getBoundKey(const LocalParam &param) {
  auto boundKey = 1.0;
  return boundKey;
}

NoiseKPZ NoiseKPZ::evalEncryptPk(const LocalParam &param) {
  auto boundErr = getBoundErr(param);
  auto boundKey = getBoundKey(param);
  auto expansionFactor = getExpansionFactor(param);

  double fresh = boundErr * (1. + 2. * expansionFactor * boundKey);
  return NoiseKPZ::of(fresh);
}

NoiseKPZ NoiseKPZ::evalAdd(const NoiseKPZ &lhs, const NoiseKPZ &rhs) {
  return NoiseKPZ::of(lhs.getValue() + rhs.getValue() + 1);
}
NoiseKPZ NoiseKPZ::evalMultNoRelin(const LocalParam &resultParam,
                                   const NoiseKPZ &lhs, const NoiseKPZ &rhs) {
  auto t = resultParam.getSchemeParam()->t;
  auto expansionFactor = getExpansionFactor(resultParam);

  return NoiseKPZ::of((expansionFactor * t / 2) *
                      (lhs.getValue() * rhs.getValue() * 2 + lhs.getValue() +
                       rhs.getValue() + 1));
}

NoiseKPZ NoiseKPZ::evalModReduce(const LocalParam &inputParam,
                                 const NoiseKPZ &input) {
  auto cv = inputParam.getDimension();
  assert(cv == 2);
  double modulus = 1L << inputParam.getSchemeParam()->qi[inputParam.getLevel()];

  auto expansionFactor = getExpansionFactor(inputParam);
  auto boundKey = getBoundKey(inputParam);

  auto scaled = input.getValue() / modulus;
  auto added = (1.0 + expansionFactor * boundKey) / 2;
  return NoiseKPZ::of(scaled + added);
}

NoiseKPZ NoiseKPZ::evalRelinearizeBV(const LocalParam &inputParam,
                                     const NoiseKPZ &input) {
  auto numDigit =
      inputParam.getSchemeParam()->numDigit(inputParam.getLevel(), false);
  auto beta = inputParam.getSchemeParam()->digit();
  auto expansionFactor = getExpansionFactor(inputParam);
  auto boundErr = getBoundErr(inputParam);

  auto boundKeySwitch = numDigit * beta * expansionFactor * boundErr / 2.0;

  return NoiseKPZ::of(input.getValue() + boundKeySwitch);
}

NoiseKPZ NoiseKPZ::evalRelinearizeHYBRID(const LocalParam &inputParam,
                                         const NoiseKPZ &input) {
  auto numDigit =
      inputParam.getSchemeParam()->numDigit(inputParam.getLevel(), true);
  auto beta = inputParam.getSchemeParam()->digit();
  auto expansionFactor = getExpansionFactor(inputParam);
  auto boundErr = getBoundErr(inputParam);
  auto boundKey = getBoundKey(inputParam);

  auto boundKeySwitch = numDigit * beta * expansionFactor * boundErr / 2.0;
  // TODO: assert boundKeySwitch <= logQlP
  // actually if > logQlP then scaled > logQl

  auto scaled = boundKeySwitch / inputParam.getSchemeParam()->P();

  // pi times, mod down
  auto added = inputParam.getSchemeParam()->pi.size() *
               (1.0 + expansionFactor * boundKey) / 2;

  return NoiseKPZ::of(input.getValue() + scaled + added);
}

NoiseKPZ NoiseKPZ::evalRelinearize(const LocalParam &inputParam,
                                   const NoiseKPZ &input) {
  if (inputParam.getSchemeParam()->dnum == 0) {
    return NoiseKPZ::evalRelinearizeBV(inputParam, input);
  }
  return NoiseKPZ::evalRelinearizeHYBRID(inputParam, input);
}

NoiseKPZ NoiseKPZ::evalRotate(const LocalParam &inputParam,
                              const NoiseKPZ &input) {
  return NoiseKPZ::evalRelinearize(inputParam, input);
}

}  // namespace heir
}  // namespace mlir
