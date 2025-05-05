#include "lib/Analysis/NoiseAnalysis/BGV/NoiseBySymbolCoeffModel.h"

#include <cassert>
#include <cmath>
#include <iomanip>
#include <ios>
#include <limits>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <string>

#include "lib/Utils/MathUtils.h"

namespace mlir {
namespace heir {
namespace bgv {

static double gaussianBound(double alpha, double ringDim, double variance) {
  double bound = sqrt(2.0 * variance) * erfinv(pow(1.0 - alpha, 1.0 / ringDim));
  return log2(bound);
}

static double laplaceBound(double alpha, double ringDim, double variance) {
  // note the natural log here
  double bound = -sqrt(variance / 2.0) * (log(alpha / ringDim));
  return log2(bound);
}

static double generalizedNormalGetBeta(double kurtosis) {
  // Function: f(beta) = gamma(5/beta)*gamma(1/beta)/gamma(3/beta)^2 - k
  auto f = [=](double beta) -> double {
    if (beta == 0) return std::numeric_limits<double>::infinity();
    double term1 = std::tgamma(5.0 / beta);
    double term2 = std::tgamma(1.0 / beta);
    double denominator = std::pow(std::tgamma(3.0 / beta), 2);
    return (term1 * term2 / denominator) - kurtosis;
  };
  // Use bisection to find the root of f(beta)=0.
  double lower = 0.05;
  double upper = 3;
  double fLower = f(lower);
  double fUpper = f(upper);

  // Expand interval until a sign change is found.
  while (fLower * fUpper > 0) {
    lower /= 2;
    upper *= 2;
    fLower = f(lower);
    fUpper = f(upper);
    if (lower < 1e-14 || upper > 1e8)
      throw std::runtime_error(
          "Failed to bracket the root in generalizedNormalGetBeta.");
  }

  const double tol = 1e-10;
  int maxIterations = 100;
  double mid = 0.0;
  for (int i = 0; i < maxIterations; ++i) {
    mid = (lower + upper) / 2;
    double fMid = f(mid);
    if (std::fabs(fMid) < tol || (upper - lower) < tol) return mid;
    if (fMid * fLower < 0) {
      upper = mid;
      fUpper = fMid;
    } else {
      lower = mid;
      fLower = fMid;
    }
  }
  throw std::runtime_error("Bisection method did not converge");
}

static double generaliedNormalGetShape(double variance, double beta) {
  return sqrt(variance * std::tgamma(1. / beta) / std::tgamma(3. / beta));
}

static double generalizedNormalBound(double alpha, double ringDim,
                                     double variance,
                                     const std::vector<int> &degreeVec) {
  double term = 1.0;
  auto binomial = [](int n, int k) {
    if (k > n) return 0.0;
    if (k == 0 || k == n) return 1.0;
    double res = 1.0;
    for (int i = 0; i < k; ++i) {
      res *= (n - i);
      res /= (i + 1);
    }
    return res;
  };
  for (auto degree : degreeVec) {
    term *= binomial(2 * degree, degree);
  }
  auto kurtosis = 3. + 3.0 * (term - 2) / ringDim;
  auto beta = generalizedNormalGetBeta(kurtosis);
  auto shape = generaliedNormalGetShape(variance, beta);
  // note the natural log here
  auto bound = shape * pow(log(ringDim / alpha), 1. / beta);
  return log2(bound);
}

double NoiseBySymbolCoeffModel::toLogBound(const LocalParamType &param,
                                           const StateType &noise) const {
  double alpha = pow(2.0, -32);
  auto ringDim = param.getSchemeParam()->getRingDim();
  auto variance = noise.toVariance(ringDim);
  if (boundType == GAUSSIAN) {
    return gaussianBound(alpha, ringDim, variance);
  }
  if (boundType == LAPLACIAN) {
    return laplaceBound(alpha, ringDim, variance);
  }
  return generalizedNormalBound(alpha, ringDim, variance,
                                noise.getDegreeForGeneralizedNormal());
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
  auto t = param.getSchemeParam()->getPlaintextModulus();
  Symbol ei("e" + std::to_string(index));
  Symbol s("s");
  Symbol es("es");
  Symbol ui("u" + std::to_string(index));
  auto ei_s = Monomial::multiply(ei, s);
  auto es_ui = Monomial::multiply(es, ui);
  auto ei_s_es_ui = Expression::multiply(Expression::add(ei_s, es_ui), t);
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
    const StateType &rhs) const {
  // return lhs.multiply(rhs, resultParam);
  return Expression::multiply(lhs, rhs);
}

}  // namespace bgv
}  // namespace heir
}  // namespace mlir
