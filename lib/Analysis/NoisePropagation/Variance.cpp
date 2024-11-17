#include "lib/Analysis/NoisePropagation/Variance.h"

#include <cmath>

#include "llvm/include/llvm/Support/raw_ostream.h"  // from @llvm-project

namespace mlir {
namespace heir {

std::map<ParamsFactory::ParamKey, Param> ParamsFactory::AllParams = {};

std::set<VarianceKey> VarianceKeyFactory::AllVarianceKeys = {};

std::string Variance::toString() const {
  switch (varianceType) {
    case (VarianceType::UNINITIALIZED):
      return "Variance(uninitialized)";
    case (VarianceType::UNBOUNDED):
      return "Variance(unbounded)";
    case (VarianceType::SET):
      return "Variance(" + std::to_string(log(getValue()) / log(2)) + ") ";
  }
}

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

double Variance::alphaBound(int n) const {
  double alpha = 0.001;
  double bound =
      sqrt(2.0 * Variance::getValue()) * erfinv(pow(1.0 - alpha, 1.0 / n));
  return bound;
}

llvm::raw_ostream &operator<<(llvm::raw_ostream &os, const Variance &variance) {
  return os << variance.toString();
}

llvm::raw_ostream &operator<<(llvm::raw_ostream &os, const VarianceKey &key) {
  key.print(os);
  return os;
}

llvm::raw_ostream &operator<<(llvm::raw_ostream &os,
                              const VarianceValues &values) {
  values.print(os);
  return os;
}

llvm::raw_ostream &operator<<(llvm::raw_ostream &os,
                              const VarianceStates &variance) {
  variance.print(os);
  return os;
}

Diagnostic &operator<<(Diagnostic &diagnostic, const Variance &variance) {
  return diagnostic << variance.toString();
}

// though for normal distro
// used for s for now...
// https://math.stackexchange.com/questions/1917647/proving-ex4-3%CF%834
// E[Xi^2n] = (2n - 1)!! Var(Xi)^n
// Var[Xi^2] = 2 Var[Xi]^2
// Var[Xi^3] = 15 Var(Xi)^3
// Var[Xi^4] = 96 Var(Xi)^4
int VariancePower(int cv) {
  if (cv == 0) {
    return 1;
  }
  // Var(Xi^n) = E[Xi^2n] - E[Xi^n]^2
  auto doubleFactorial = [](int n) {
    int ret = 1;
    for (n = n - 1; n > 0; n -= 2) {
      ret *= n;
    }
    return ret;
  };
  int termLeft = doubleFactorial(cv * 2);
  int termRight = 0;
  if (cv % 2 == 0) {
    int E = doubleFactorial(cv);
    termRight = E * E;
  }
  // E[Xi^odd] = 0
  return termLeft - termRight;
}

Variance Variance::evalEncryptPk(double n, double t, double std0) {
  double variance0 = std0 * std0;
  // assumed UNIFORM_TENARY
  double encrypt = variance0 * t * t * (4.0 * n / 3 + 1);
  // major error independent (public key e is not dominant)
  return Variance::of(encrypt, "s", 1);
}

Variance Variance::evalAdd(const Variance &lhs, const Variance &rhs) {
  return lhs + rhs;
}
Variance Variance::evalMultNoRelin(const Variance &lhs, const Variance &rhs,
                                   double n, double t) {
  double power = 1.0;
  VarianceMajorFactor factor;
  auto lhsFactor = lhs.getFactor();
  auto rhsFactor = rhs.getFactor();
  if (lhsFactor.correlate(rhsFactor)) {
    // in factor we need strict covariance analysis
    power = lhsFactor.getPower(rhsFactor);
    // always track s
    if (lhsFactor.reason == "s") {
      factor = lhsFactor.merge(rhsFactor);
    } else {
      factor = VarianceMajorFactor("s", 2);
    }
    // discard other small variable
  } else {
    // always reset to s^2
    // TODO: replace 2 with result.cv - 1
    factor = VarianceMajorFactor("s", 2);
  }

  // component m_i uniform mod t, giving |m| \approx n*(t^2-1)/12
  return Variance::of(power * lhs.getValue() * rhs.getValue() * n +
                          lhs.getValue() * n * (t * t - 1) / 12 +
                          rhs.getValue() * n * (t * t - 1) / 12,
                      factor);
}

Variance Variance::evalModUp(const Variance &input, double modulus, double n,
                             double t, int cv) {
  double sVarianceTerm = 1.0;         // s^0
  double sVariances = sVarianceTerm;  // total
  // assumed UNIFORM_TENARY
  double sVariance = 2.0 / 3;
  for (int cv_index = 1; cv_index != cv; ++cv_index) {
    // check corollary 1 of [MP24]
    sVarianceTerm *= sVariance * n * VariancePower(cv_index);
    sVariances += sVarianceTerm;
  }
  double added = 1.0 / 12 * t * t * sVariances;
  return Variance::of(input.getValue() * (modulus * modulus) + added,
                      input.getFactor());
}

Variance Variance::evalModReduce(const Variance &input, double modulus,
                                 double n, double t, int cv) {
  double sVarianceTerm = 1.0;         // s^0
  double sVariances = sVarianceTerm;  // total
  // assumed UNIFORM_TENARY
  double sVariance = 2.0 / 3;
  for (int cv_index = 1; cv_index != cv; ++cv_index) {
    // check corollary 1 of [MP24]
    // LLVM_DEBUG(llvm::dbgs() << "cv " << cv_index << " power " <<
    // VariancePower(cv_index) << "\n");
    sVarianceTerm *= sVariance * n * VariancePower(cv_index);
    sVariances += sVarianceTerm;
  }
  double added = 1.0 / 12 * t * t * sVariances;
  double scaled = input.getValue() / (modulus * modulus);
  VarianceMajorFactor factor;
  if (scaled > added) {
    factor = input.getFactor();
  } else {
    factor = VarianceMajorFactor("s", cv - 1);
  }
  return Variance::of(scaled + added, factor);
}

Variance Variance::evalRelinearizeBV(const Variance &input, double n, double t,
                                     double std0, double numDigit,
                                     double beta) {
  double variance0 = std0 * std0;
  double term1 = variance0 * t * t * n / 12.0;
  double term2 = numDigit * beta * beta;
  double inherent = input.getValue();
  double added = term1 * term2;
  VarianceMajorFactor factor;
  if (inherent > added) {
    factor = input.getFactor();
  } else {
    // relinearize major
    factor = VarianceMajorFactor("s", 2);
  }
  return Variance::of(inherent + added, factor);
}

// Variance Variance::evalRotate(const Variance &input, double n, double t,
// double std0, double numDigit, double beta) {
//     return Variance::evalRelinearize(input, n, t, std0, numDigit, beta);
// }

std::string VarianceValues::toDOTEdge(
    std::vector<Value> values, const DenseMap<Value, std::string> &valueNameMap,
    const std::vector<std::tuple<VarianceKey, VarianceParents>> &selected)
    const {
  std::string str;
  for (auto &p : v) {
    auto &variance = std::get<0>(p);
    auto &parents = std::get<1>(p);

    bool markRed = false;
    for (auto &[curKey, curParents] : selected) {
      if (*k == curKey && parents == curParents) {
        markRed = true;
      }
    }
    if (!markRed) continue;

    for (auto &parent : parents.getParents()) {
      std::string valueName = valueNameMap.at(values[0]);
      std::string parentValueName = valueName;
      if (parent.type != VarianceParentType::Self) {
        auto parentValue = values[1];
        if (parent.type == VarianceParentType::Operand1)
          parentValue = values[2];
        parentValueName = valueNameMap.at(parentValue);
      }

      str += parent.parentKey->toDOTNode(parentValueName) + " -> " +
             k->toDOTNode(valueName) + " [label=\"" +
             variance.toBound(k->p->n) + " " + parents.reason + " " +
             std::to_string(int(parents.cost)) + "\"";
      if (markRed && variance.isBounded()) {
        str += " color=red fontcolor=red";
      } else {
        str += " color=gray fontcolor=gray";
        if (!variance.isBounded()) {
          str += " style=dashed";
        }
      }
      str += "]\n";
    }
  }
  return str;
}

}  // namespace heir
}  // namespace mlir
