#ifndef INCLUDE_ANALYSIS_NOISEPROPAGATION_VARIANCE_H_
#define INCLUDE_ANALYSIS_NOISEPROPAGATION_VARIANCE_H_

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <optional>

#include "llvm/include/llvm/Support/raw_ostream.h"  // from @llvm-project
#include "mlir/include/mlir/IR/Diagnostics.h"       // from @llvm-project

namespace mlir {
namespace heir {

enum VarianceType {
  // A min value for the lattice, discarable when joined with anything else.
  UNINITIALIZED,
  // A known value for the lattice, when noise can be inferred.
  SET,
  // A max value for the lattice, when noise cannot be inferred and a bootstrap
  // must be forced.
  UNBOUNDED
};

/// A class representing an optional variance of a noise distribution.
class Variance {
 public:
  static Variance uninitialized() {
    return Variance(VarianceType::UNINITIALIZED, std::nullopt);
  }
  static Variance unbounded() {
    return Variance(VarianceType::UNBOUNDED, std::nullopt);
  }
  static Variance of(double value) {
    return Variance(VarianceType::SET, value);
  }

  /// Create an integer value range lattice value.
  /// The default constructor must be equivalent to the "entry state" of the
  /// lattice, i.e., an uninitialized noise variance.
  Variance(VarianceType varianceType = VarianceType::UNINITIALIZED,
           std::optional<double> value = std::nullopt)
      : varianceType(varianceType), value(value) {}

  bool isKnown() const { return varianceType == VarianceType::SET; }

  bool isInitialized() const {
    return varianceType != VarianceType::UNINITIALIZED;
  }

  bool isBounded() const { return varianceType != VarianceType::UNBOUNDED; }

  const double &getValue() const {
    assert(isKnown());
    return *value;
  }

  bool operator==(const Variance &rhs) const {
    return varianceType == rhs.varianceType && value == rhs.value;
  }

  Variance operator+(const Variance &rhs) const {
    return (this->isBounded() && rhs.isBounded())
               ? Variance::of(this->getValue() + rhs.getValue())
               : Variance::unbounded();
  }

  Variance operator*(const Variance &rhs) const {
    return (this->isBounded() && rhs.isBounded())
               ? Variance::of(this->getValue() + rhs.getValue())
               : Variance::unbounded();
  }

  Variance max(const Variance &rhs) const {
    return (this->isBounded() && rhs.isBounded())
               ? Variance::of(std::max(this->getValue(), rhs.getValue()))
               : Variance::unbounded();
  }

  static Variance join(const Variance &lhs, const Variance &rhs) {
    // Uninitialized variances correspond to values that are not secret,
    // which may be the inputs to an encryption operation.
    if (lhs.varianceType == VarianceType::UNINITIALIZED) {
      return rhs;
    }
    if (rhs.varianceType == VarianceType::UNINITIALIZED) {
      return lhs;
    }

    // Unbounded represents a pessimistic worst case, and so it must be
    // preserved no matter the other operand.
    if (lhs.varianceType == VarianceType::UNBOUNDED) {
      return lhs;
    }
    if (rhs.varianceType == VarianceType::UNBOUNDED) {
      return rhs;
    }

    assert(lhs.varianceType == VarianceType::SET &&
           rhs.varianceType == VarianceType::SET);
    return Variance::of(std::max(lhs.getValue(), rhs.getValue()));
  }

  // std0: std error of e distribution
  // assumed UNIFORM_TENARY secret distribution
  static Variance evalEncryptPk(double n, double t, double std0);
  static Variance evalAdd(const Variance &lhs, const Variance &rhs);
  static Variance evalMultNoRelin(const Variance &lhs, const Variance &rhs,
                                  double n);
  // l: number of digit
  // beta: base
  static Variance evalModUp(const Variance &input, double modulus, double n,
                            double t);
  static Variance evalRelinearizeBV(const Variance &input, double n, double t,
                                    double std0, double numDigit, double beta);
  static Variance evalModReduce(const Variance &input, double modulus, double n,
                                double t);
  // static Variance evalRotate(const Variance &input, double n, double t,
  // double std0, double numDigit, double beta);

  double alphaBound(int n) const;

  void print(llvm::raw_ostream &os) const { os << value; }

  std::string toString() const;

  friend llvm::raw_ostream &operator<<(llvm::raw_ostream &os,
                                       const Variance &variance);

  friend Diagnostic &operator<<(Diagnostic &diagnostic,
                                const Variance &variance);

 private:
  VarianceType varianceType;
  std::optional<double> value;
};

class VarianceState {
 public:
  VarianceState(int n, int t, int cv, int l, Variance variance)
      : n(n), t(t), cv(cv), l(l), variance(variance) {}

  void print(llvm::raw_ostream &os) const {
    os << variance << " (" << n << " " << t << " " << cv << " " << l << " )";
  }

  bool sameState(const VarianceState &rhs) const {
    return n == rhs.n && t == rhs.t && cv == rhs.cv && l == rhs.l;
  }

  bool operator==(const VarianceState &rhs) const {
    return sameState(rhs) && variance == rhs.variance;
  }

  static VarianceState join(const VarianceState &lhs,
                            const VarianceState &rhs) {
    assert(lhs.sameState(rhs));
    return VarianceState(lhs.n, lhs.t, lhs.cv, lhs.l,
                         Variance::join(lhs.variance, rhs.variance));
  }

 private:
  int n;
  int t;
  int cv;
  int l;
  Variance variance;
};

class VarianceStates {
 public:
  VarianceStates() = default;
  VarianceStates(std::vector<VarianceState> states) : states(states) {}

  void print(llvm::raw_ostream &os) const {
    os << '[';
    for (auto &s : states) {
      s.print(os);
    }
    os << ']';
  }

  bool operator==(const VarianceStates &rhs) const {
    for (auto &l : states) {
      for (auto &r : rhs.states) {
        if (l.sameState(r) && !(l == r)) {
          return false;
        }
      }
    }
    return true;
  }

  static VarianceStates join(const VarianceStates &lhs,
                             const VarianceStates &rhs) {
    std::vector<VarianceState> res;
    for (auto &l : lhs.states) {
      for (auto &r : rhs.states) {
        if (l.sameState(r)) {
          res.push_back(VarianceState::join(l, r));
        }
      }
    }
    return VarianceStates(res);
  }

 private:
  std::vector<VarianceState> states;
};

}  // namespace heir
}  // namespace mlir

#endif  // INCLUDE_ANALYSIS_NOISEPROPAGATION_VARIANCE_H_
