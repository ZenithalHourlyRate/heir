#include "lib/Analysis/NoiseAnalysis/BFV/NoiseByVarianceCoeffModel.h"

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

// the formulae below are mainly taken from BMCM23
// "Improving and Automating BFV Parameters Selection: An Average-Case Approach"
// https://ia.cr/2023/600
// with modification that it works on ||e|| instead of invariant noise

template <bool P>
using Model = NoiseByVarianceCoeffModel<P>;

// https://stackoverflow.com/questions/27229371/inverse-error-function-in-c
static double erfinv(double a) {
  double p, r, t;
  t = fma(a, 0.0 - a, 1.0);
  t = log(t);
  if (fabs(t) > 6.125) {            // maximum ulp error = 2.35793
    p = 3.03697567e-10;             //  0x1.4deb44p-32
    p = fma(p, t, 2.93243101e-8);   //  0x1.f7c9aep-26
    p = fma(p, t, 1.22150334e-6);   //  0x1.47e512p-20
    p = fma(p, t, 2.84108955e-5);   //  0x1.dca7dep-16
    p = fma(p, t, 3.93552968e-4);   //  0x1.9cab92p-12
    p = fma(p, t, 3.02698812e-3);   //  0x1.8cc0dep-9
    p = fma(p, t, 4.83185798e-3);   //  0x1.3ca920p-8
    p = fma(p, t, -2.64646143e-1);  // -0x1.0eff66p-2
    p = fma(p, t, 8.40016484e-1);   //  0x1.ae16a4p-1
  } else {                          // maximum ulp error = 2.35002
    p = 5.43877832e-9;              //  0x1.75c000p-28
    p = fma(p, t, 1.43285448e-7);   //  0x1.33b402p-23
    p = fma(p, t, 1.22774793e-6);   //  0x1.499232p-20
    p = fma(p, t, 1.12963626e-7);   //  0x1.e52cd2p-24
    p = fma(p, t, -5.61530760e-5);  // -0x1.d70bd0p-15
    p = fma(p, t, -1.47697632e-4);  // -0x1.35be90p-13
    p = fma(p, t, 2.31468678e-3);   //  0x1.2f6400p-9
    p = fma(p, t, 1.15392581e-2);   //  0x1.7a1e50p-7
    p = fma(p, t, -2.32015476e-1);  // -0x1.db2aeep-3
    p = fma(p, t, 8.86226892e-1);   //  0x1.c5bf88p-1
  }
  r = a * p;
  return r;
}

template <bool P>
double Model<P>::toLogBound(const LocalParamType &param,
                            const StateType &noise) {
  // error probability 0.1%
  // though this only holds if every random variable is Gaussian
  // or similar to Gaussian
  // so this may give underestimation, see MP24 and CCH+23
  double alpha = 0.001;
  auto ringDim = param.getSchemeParam()->getRingDim();
  double bound =
      sqrt(2.0 * noise.getValue()) * erfinv(pow(1.0 - alpha, 1.0 / ringDim));
  return log2(bound);
}

template <bool P>
double Model<P>::toLogBudget(const LocalParamType &param,
                             const StateType &noise) {
  return toLogTotal(param) - toLogBound(param, noise);
}

template <bool P>
double Model<P>::toLogTotal(const LocalParamType &param) {
  double total = 0;
  auto logqi = param.getSchemeParam()->getLogqi();
  for (auto i = 0; i <= param.getCurrentLevel(); ++i) {
    total += logqi[i];
  }
  double logT = log2(param.getSchemeParam()->getPlaintextModulus());
  return total - logT - 1.0;
}

template <bool P>
std::string Model<P>::toLogBoundString(const LocalParamType &param,
                                       const StateType &noise) {
  auto logBound = toLogBound(param, noise);
  std::stringstream stream;
  stream << std::fixed << std::setprecision(2) << logBound;
  return stream.str();
}

template <bool P>
std::string Model<P>::toLogBudgetString(const LocalParamType &param,
                                        const StateType &noise) {
  auto logBudget = toLogBudget(param, noise);
  std::stringstream stream;
  stream << std::fixed << std::setprecision(2) << logBudget;
  return stream.str();
}

template <bool P>
std::string Model<P>::toLogTotalString(const LocalParamType &param) {
  auto logTotal = toLogTotal(param);
  std::stringstream stream;
  stream << std::fixed << std::setprecision(2) << logTotal;
  return stream.str();
}

template <bool P>
double Model<P>::getVarianceErr(const LocalParamType &param) {
  auto std0 = param.getSchemeParam()->getStd0();
  return std0 * std0;
}

template <bool P>
double Model<P>::getVarianceKey(const LocalParamType &param) {
  // assume UNIFORM_TERNARY
  return 2.0 / 3.0;
}

template <bool P>
typename Model<P>::StateType Model<P>::evalEncryptPk(
    const LocalParamType &param) {
  auto varianceError = getVarianceErr(param);
  // uniform ternary
  auto varianceKey = getVarianceKey(param);
  auto n = param.getSchemeParam()->getRingDim();
  // public key (-as + e, a)
  // public key encryption (-aus + (u * e + e_0) + (q/t) * m, au + e_1)
  // v_fresh = u * e + e_1 * s + e_0
  // var_fresh = (2n * var_key + 1) * var_error
  // for ringDim, see header comment for explanation
  double fresh = varianceError * (2. * n * varianceKey + 1.);
  return StateType::of(fresh);
}

template <bool P>
typename Model<P>::StateType Model<P>::evalEncryptSk(
    const LocalParamType &param) {
  auto varianceError = getVarianceErr(param);

  // secret key s
  // secret key encryption (-as + (q/t) * m + e, a)
  // v_fresh = e
  // var_fresh = var_error
  double fresh = varianceError;
  return StateType::of(fresh);
}

template <bool P>
typename Model<P>::StateType Model<P>::evalEncrypt(
    const LocalParamType &param) {
  // P stands for public key encryption
  if constexpr (P) {
    return evalEncryptPk(param);
  } else {
    return evalEncryptSk(param);
  }
}

template <bool P>
typename Model<P>::StateType Model<P>::evalConstant(
    const LocalParamType &param) {
  auto t = param.getSchemeParam()->getPlaintextModulus();
  // constant is v = (q/t)m
  // due to B/FV multiplicatoin we will rescale by (q/t) after each
  // multiplication so the variance that is effective on the error is v = m
  // assume m is uniform from [-t/2, t/2]
  // var_constant = t * t / 12
  return StateType::of(t * t / 12.0);
}

template <bool P>
typename Model<P>::StateType Model<P>::evalAdd(const StateType &lhs,
                                               const StateType &rhs) {
  // v_add = v_0 + v_1
  // assuming independent of course
  return StateType::of(lhs.getValue() + rhs.getValue());
}

template <bool P>
typename Model<P>::StateType Model<P>::evalMul(
    const LocalParamType &resultParam, const StateType &lhs,
    const StateType &rhs) {
  auto ringDim = resultParam.getSchemeParam()->getRingDim();
  auto v0 = lhs.getValue();
  auto v1 = rhs.getValue();

  auto m0 = evalConstant(resultParam).getValue();
  auto m1 = evalConstant(resultParam).getValue();

  // ((q/t)m_0 + e_0) * ((q/t)m_1 + e_1)
  // = (q/t)^2 m_0 * m_1 + (q/t)(m_0 e_1 + m_1 * e_0) + e_0 * e_1
  // after scaling by (q/t) we get
  // (q/t) m_0 * m_1 + (m_0 * e_1 + m_1 * e_0) + (t/q)e_0 * e_1
  // v_mul = v_0 * v_1 * (t/q) + v_0 * m_1 + v_1 * m_0
  // we can drop v_0 * v_1 * (t/q) as it is _often_ negligible
  // for ringDim, see header comment for explanation
  return StateType::of(ringDim * (v0 * m1 + v1 * m0));
}

template <bool P>
typename Model<P>::StateType Model<P>::evalRelinearizeHYBRID(
    const LocalParamType &inputParam, const StateType &input) {
  return StateType::of(input.getValue());
}

template <bool P>
typename Model<P>::StateType Model<P>::evalRelinearize(
    const LocalParamType &inputParam, const StateType &input) {
  // assume HYBRID
  // if we further introduce BV to SchemeParam we can have alternative
  // implementation.
  return evalRelinearizeHYBRID(inputParam, input);
}

// instantiate template class
template class NoiseByVarianceCoeffModel<false>;
template class NoiseByVarianceCoeffModel<true>;

}  // namespace bfv
}  // namespace heir
}  // namespace mlir
