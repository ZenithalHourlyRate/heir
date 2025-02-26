#include "lib/Analysis/NoiseAnalysis/BFV/NoiseByBoundCoeffModel.h"

#include <cassert>
#include <cmath>
#include <iomanip>
#include <ios>
#include <numeric>
#include <sstream>
#include <string>

namespace mlir {
namespace heir {
namespace bfv {

// the formulae below are mainly taken from KPZ21
// "Revisiting Homomorphic Encryption Schemes for Finite Fields"
// https://ia.cr/2021/204

template <bool W, bool P>
using Model = NoiseByBoundCoeffModel<W, P>;

template <bool W, bool P>
double Model<W, P>::toLogBound(const LocalParamType &param,
                               const StateType &noise) {
  return log(noise.getValue()) / log(2);
}

template <bool W, bool P>
double Model<W, P>::toLogBudget(const LocalParamType &param,
                                const StateType &noise) {
  return toLogTotal(param) - toLogBound(param, noise);
}

template <bool W, bool P>
double Model<W, P>::toLogTotal(const LocalParamType &param) {
  double total = 0;
  auto logqi = param.getSchemeParam()->getLogqi();
  for (auto i = 0; i <= param.getSchemeParam()->getLevel(); ++i) {
    total += logqi[i];
  }
  double logT = log2(param.getSchemeParam()->getPlaintextModulus());
  return total - logT - 1.0;
}

template <bool W, bool P>
std::string Model<W, P>::toLogBoundString(const LocalParamType &param,
                                          const StateType &noise) {
  auto logBound = toLogBound(param, noise);
  std::stringstream stream;
  stream << std::fixed << std::setprecision(2) << logBound;
  return stream.str();
}

template <bool W, bool P>
std::string Model<W, P>::toLogBudgetString(const LocalParamType &param,
                                           const StateType &noise) {
  auto logBudget = toLogBudget(param, noise);
  std::stringstream stream;
  stream << std::fixed << std::setprecision(2) << logBudget;
  return stream.str();
}

template <bool W, bool P>
std::string Model<W, P>::toLogTotalString(const LocalParamType &param) {
  auto logTotal = toLogTotal(param);
  std::stringstream stream;
  stream << std::fixed << std::setprecision(2) << logTotal;
  return stream.str();
}

template <bool W, bool P>
double Model<W, P>::getExpansionFactor(const LocalParamType &param) {
  auto n = param.getSchemeParam()->getRingDim();
  if constexpr (W) {
    // worst-case
    // well known from DPSZ12
    return n;
  } else {
    // average-case
    // experimental result
    // cite HPS19 and KPZ21
    return 2.0 * sqrt(n);
  }
}

template <bool W, bool P>
double Model<W, P>::getBoundErr(const LocalParamType &param) {
  auto std0 = param.getSchemeParam()->getStd0();
  // probability of larger than 6 * std0 is less than 2^{-30}
  auto assurance = 6;
  auto boundErr = std0 * assurance;
  return boundErr;
}

template <bool W, bool P>
double Model<W, P>::getBoundKey(const LocalParamType &param) {
  // assume UNIFORM_TERNARY
  auto boundKey = 1.0;
  return boundKey;
}

template <bool W, bool P>
typename Model<W, P>::StateType Model<W, P>::evalEncryptPk(
    const LocalParamType &param) {
  auto boundErr = getBoundErr(param);
  auto boundKey = getBoundKey(param);
  auto expansionFactor = getExpansionFactor(param);

  // public key (-as + e, a)
  // public key encryption (-aus + u * e + e_0 + (Q/t)m, au + e_1)
  // (Q/t)m + u * e + e_1 * s + e_0
  // v_fresh = u * e + e_1 * s + e_0
  double fresh = boundErr * (1. + 2. * expansionFactor * boundKey);
  return StateType::of(fresh);
}

template <bool W, bool P>
typename Model<W, P>::StateType Model<W, P>::evalEncryptSk(
    const LocalParamType &param) {
  auto boundErr = getBoundErr(param);

  // secret key s
  // secret key encryption (-as + (Q/t)m + e, a)
  // v_fresh = e
  double fresh = boundErr;
  return StateType::of(fresh);
}

template <bool W, bool P>
typename Model<W, P>::StateType Model<W, P>::evalEncrypt(
    const LocalParamType &param) {
  // P stands for public key encryption
  if constexpr (P) {
    return evalEncryptPk(param);
  } else {
    return evalEncryptSk(param);
  }
}

template <bool W, bool P>
typename Model<W, P>::StateType Model<W, P>::evalConstant(
    const LocalParamType &param) {
  // constant is (Q/t)m + 0
  // v_constant = 0
  return StateType::of(0);
}

template <bool W, bool P>
typename Model<W, P>::StateType Model<W, P>::evalAdd(const StateType &lhs,
                                                     const StateType &rhs) {
  // (Q/t)m_0 + v_0 + (Q/t)m_1 + v_1 <= (Q/t)[m_0 + m_1]_t + (v_0 + v_1 + r(Q)u)
  // mod Q v_add = v_0 + v_1 + r(Q)u
  // where ||u|| <= 1
  return StateType::of(lhs.getValue() + rhs.getValue() + 1);
}

template <bool W, bool P>
typename Model<W, P>::StateType Model<W, P>::evalMul(
    const LocalParamType &resultParam, const StateType &lhs,
    const StateType &rhs) {
  auto expansionFactor = getExpansionFactor(resultParam);
  auto t = resultParam.getSchemeParam()->getPlaintextModulus();
  auto v0 = lhs.getValue();
  auto v1 = rhs.getValue();
  auto boundKey = getBoundKey(resultParam);

  auto logqi = resultParam.getSchemeParam()->getLogqi();
  auto logQ = std::accumulate(logqi.begin(), logqi.end(), 0.0);
  // we hope double is big enough...
  // if logQ > 1024... we may have a problem
  auto Q = pow(2.0, logQ);

  // See KPZ21
  auto term1 = expansionFactor * t * v0 * v1 / Q;
  auto term2 = (expansionFactor * t / 2.0) *
               (4.0 + expansionFactor * boundKey) * (v0 + v1);
  auto term3 = (1.0 + expansionFactor * boundKey +
                expansionFactor * expansionFactor * boundKey * boundKey) /
               2.0;
  return StateType::of(term1 + term2 + term3);
}

template <bool W, bool P>
typename Model<W, P>::StateType Model<W, P>::evalRelinearizeHYBRID(
    const LocalParamType &inputParam, const StateType &input) {
  return StateType::of(input.getValue());
}

template <bool W, bool P>
typename Model<W, P>::StateType Model<W, P>::evalRelinearize(
    const LocalParamType &inputParam, const StateType &input) {
  // assume HYBRID
  // if we further introduce BV to SchemeParam we can have alternative
  // implementation.
  return evalRelinearizeHYBRID(inputParam, input);
}

// instantiate template class
template class NoiseByBoundCoeffModel<false, true>;
template class NoiseByBoundCoeffModel<true, true>;
template class NoiseByBoundCoeffModel<false, false>;
template class NoiseByBoundCoeffModel<true, false>;

}  // namespace bfv
}  // namespace heir
}  // namespace mlir
