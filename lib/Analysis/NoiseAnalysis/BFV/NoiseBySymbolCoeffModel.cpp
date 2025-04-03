#include "lib/Analysis/NoiseAnalysis/BFV/NoiseBySymbolCoeffModel.h"

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

// the formulae below are mainly taken from MP24
// "A Central Limit Framework for Ring-LWE Noise Analysis"
// https://ia.cr/2019/452
// and CLP23
// "Optimisations and tradeoffs for HElib"
// https://ia.cr/2023/104

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

double NoiseBySymbolCoeffModel::toLogBound(const LocalParamType &param,
                                           const StateType &noise) const {
  // error probability 0.1%
  // though this only holds if every random variable is Gaussian
  // or similar to Gaussian
  // so this may give underestimation, see MP24 and CCH+23
  double alpha = pow(2.0, -32);
  auto ringDim = param.getSchemeParam()->getRingDim();
  double bound =
      -sqrt(noise.toVariance(ringDim) / 2.0) * (log(alpha / ringDim));
  return log2(bound);
}

double NoiseBySymbolCoeffModel::toLogBudget(const LocalParamType &param,
                                            const StateType &noise) const {
  return toLogTotal(param) - toLogBound(param, noise);
}

double NoiseBySymbolCoeffModel::toLogTotal(const LocalParamType &param) const {
  double total = 0;
  auto logqi = param.getSchemeParam()->getLogqi();
  for (auto i = 0; i <= param.getCurrentLevel(); ++i) {
    total += logqi[i];
  }
  return total - 1.0;
}

std::string NoiseBySymbolCoeffModel::toLogBoundString(
    const LocalParamType &param, const StateType &noise) const {
  auto logBound = toLogBound(param, noise);
  std::stringstream stream;
  stream << std::fixed << std::setprecision(2) << logBound;
  return stream.str();
}

std::string NoiseBySymbolCoeffModel::toLogBudgetString(
    const LocalParamType &param, const StateType &noise) const {
  auto logBudget = toLogBudget(param, noise);
  std::stringstream stream;
  stream << std::fixed << std::setprecision(2) << logBudget;
  return stream.str();
}

std::string NoiseBySymbolCoeffModel::toLogTotalString(
    const LocalParamType &param) const {
  auto logTotal = toLogTotal(param);
  std::stringstream stream;
  stream << std::fixed << std::setprecision(2) << logTotal;
  return stream.str();
}

typename NoiseBySymbolCoeffModel::StateType
NoiseBySymbolCoeffModel::evalEncryptPk(const LocalParamType &param,
                                       unsigned index) const {
  experimental::Symbol ei("e" + std::to_string(index));
  experimental::Symbol s("s");
  experimental::Symbol es("es");
  experimental::Symbol ui("u" + std::to_string(index));
  auto ei_s = experimental::Monomial::multiply(ei, s);
  auto es_ui = experimental::Monomial::multiply(es, ui);
  auto ei_s_es_ui = experimental::Expression::add(ei_s, es_ui);
  return ei_s_es_ui;
}

// typename NoiseBySymbolCoeffModel::StateType
// NoiseBySymbolCoeffModel::evalEncryptSk(const LocalParamType &param,
//                                                unsigned index) {
//   //Symbol symbol("m" + std::to_string(index), SymbolType::EncryptPk);
//   return Expression(symbol);
// }

typename NoiseBySymbolCoeffModel::StateType
NoiseBySymbolCoeffModel::evalEncrypt(const LocalParamType &param,
                                     unsigned index) const {
  auto usePublicKey = param.getSchemeParam()->getUsePublicKey();
  if (usePublicKey) {
    return evalEncryptPk(param, index);
  }
  return evalEncryptPk(param, index);
  // return evalEncryptSk(param, index);
}

typename NoiseBySymbolCoeffModel::StateType NoiseBySymbolCoeffModel::evalMul(
    const LocalParamType &resultParam, const StateType &lhs,
    const StateType &rhs, const AnalysisSymbolState &lhsSymbol,
    const AnalysisSymbolState &rhsSymbol) const {
  auto t = resultParam.getSchemeParam()->getPlaintextModulus();
  auto k0 = experimental::Symbol(lhsSymbol.getSymbol());
  auto k1 = experimental::Symbol(rhsSymbol.getSymbol());
  // k0 * s * rhs
  experimental::Symbol s("s");
  experimental::Monomial k0_s = experimental::Monomial::multiply(k0, s);
  experimental::Monomial k1_s = experimental::Monomial::multiply(k1, s);
  experimental::Expression k0_s_rhs = experimental::Expression::multiply(
      experimental::Expression::multiply(k0_s, rhs), t);
  experimental::Expression k1_s_lhs = experimental::Expression::multiply(
      experimental::Expression::multiply(k1_s, lhs), t);
  // experimental::Expression k0_s_rhs =
  //     experimental::Expression::multiply(k0_s, rhs);
  // experimental::Expression k1_s_lhs =
  //     experimental::Expression::multiply(k1_s, lhs);
  return experimental::Expression::add(k0_s_rhs, k1_s_lhs);
}

}  // namespace bfv
}  // namespace heir
}  // namespace mlir
