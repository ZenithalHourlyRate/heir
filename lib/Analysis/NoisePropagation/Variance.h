#ifndef INCLUDE_ANALYSIS_NOISEPROPAGATION_VARIANCE_H_
#define INCLUDE_ANALYSIS_NOISEPROPAGATION_VARIANCE_H_

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <optional>

#include "llvm/include/llvm/Support/raw_ostream.h"  // from @llvm-project
#include "mlir/include/mlir/IR/Diagnostics.h"       // from @llvm-project
#include "mlir/include/mlir/IR/Value.h"             // from @llvm-project

namespace mlir {
namespace heir {

// forward declaration
class LocalParam;

/// A class representing an optional variance of a noise distribution.
class Variance {
 public:
  enum VarianceType {
    // A min value for the lattice, discarable when joined with anything else.
    UNINITIALIZED,
    // A known value for the lattice, when noise can be inferred.
    SET,
    // A max value for the lattice, when noise cannot be inferred and a
    // bootstrap
    // must be forced.
    UNBOUNDED
  };

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

  static Variance min(const Variance &lhs, const Variance &rhs) {
    if (lhs.varianceType == VarianceType::UNBOUNDED) {
      return rhs;
    }
    if (rhs.varianceType == VarianceType::UNBOUNDED) {
      return lhs;
    }

    assert(lhs.varianceType == VarianceType::SET &&
           rhs.varianceType == VarianceType::SET);
    return Variance::of(std::min(lhs.getValue(), rhs.getValue()));
  }

  // std0: std error of e distribution
  // assumed UNIFORM_TENARY secret distribution
  static Variance evalEncryptPk(const LocalParam &param);
  static Variance evalAdd(const Variance &lhs, const Variance &rhs);
  static Variance evalMultNoRelin(const LocalParam &resultParam,
                                  const Variance &lhs, const Variance &rhs);
  // l: number of digit
  // beta: base
  static Variance evalModUp(const LocalParam &inputParam,
                            const Variance &input);
  static Variance evalRelinearizeBV(const LocalParam &inputParam,
                                    const Variance &input);
  static Variance evalModReduce(const LocalParam &inputParam,
                                const Variance &input);
  // static Variance evalRotate(const Variance &input, double n, double t,
  // double std0, double numDigit, double beta);

  static Variance boundBy(const Variance &v, const LocalParam &param);

  double alphaBound(int n) const;

  double logAlphaBound(int n) const { return log(alphaBound(n)) / log(2); }

  void print(llvm::raw_ostream &os) const { os << value; }

  std::string toString() const;

  std::string toBound(const LocalParam &resultParam) const;

  friend llvm::raw_ostream &operator<<(llvm::raw_ostream &os,
                                       const Variance &variance);

  friend Diagnostic &operator<<(Diagnostic &diagnostic,
                                const Variance &variance);

 private:
  VarianceType varianceType;
  std::optional<double> value;
};

}  // namespace heir
}  // namespace mlir

#endif  // INCLUDE_ANALYSIS_NOISEPROPAGATION_VARIANCE_H_
