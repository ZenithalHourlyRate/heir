#ifndef INCLUDE_ANALYSIS_NOISEPROPAGATION_NOISEKPZ_H_
#define INCLUDE_ANALYSIS_NOISEPROPAGATION_NOISEKPZ_H_

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
class NoiseKPZ {
 public:
  enum NoiseKPZType {
    // A min value for the lattice, discarable when joined with anything else.
    UNINITIALIZED,
    // A known value for the lattice, when noise can be inferred.
    SET,
    // A max value for the lattice, when noise cannot be inferred and a
    // bootstrap
    // must be forced.
    UNBOUNDED
  };

  static NoiseKPZ uninitialized() {
    return NoiseKPZ(NoiseKPZType::UNINITIALIZED, std::nullopt);
  }
  static NoiseKPZ unbounded() {
    return NoiseKPZ(NoiseKPZType::UNBOUNDED, std::nullopt);
  }
  static NoiseKPZ of(double value) {
    return NoiseKPZ(NoiseKPZType::SET, value);
  }

  /// Create an integer value range lattice value.
  /// The default constructor must be equivalent to the "entry state" of the
  /// lattice, i.e., an uninitialized noise variance.
  NoiseKPZ(NoiseKPZType varianceType = NoiseKPZType::UNINITIALIZED,
           std::optional<double> value = std::nullopt)
      : varianceType(varianceType), value(value) {}

  bool isKnown() const { return varianceType == NoiseKPZType::SET; }

  bool isInitialized() const {
    return varianceType != NoiseKPZType::UNINITIALIZED;
  }

  bool isBounded() const { return varianceType != NoiseKPZType::UNBOUNDED; }

  const double &getValue() const {
    assert(isKnown());
    return *value;
  }

  bool operator==(const NoiseKPZ &rhs) const {
    return varianceType == rhs.varianceType && value == rhs.value;
  }

  static NoiseKPZ join(const NoiseKPZ &lhs, const NoiseKPZ &rhs) {
    // Uninitialized variances correspond to values that are not secret,
    // which may be the inputs to an encryption operation.
    if (lhs.varianceType == NoiseKPZType::UNINITIALIZED) {
      return rhs;
    }
    if (rhs.varianceType == NoiseKPZType::UNINITIALIZED) {
      return lhs;
    }

    // Unbounded represents a pessimistic worst case, and so it must be
    // preserved no matter the other operand.
    if (lhs.varianceType == NoiseKPZType::UNBOUNDED) {
      return lhs;
    }
    if (rhs.varianceType == NoiseKPZType::UNBOUNDED) {
      return rhs;
    }

    assert(lhs.varianceType == NoiseKPZType::SET &&
           rhs.varianceType == NoiseKPZType::SET);
    return NoiseKPZ::of(std::max(lhs.getValue(), rhs.getValue()));
  }

  static double getExpansionFactor(const LocalParam &param);
  static double getBoundErr(const LocalParam &param);
  static double getBoundKey(const LocalParam &param);

  // std0: std error of e distribution
  // assumed UNIFORM_TENARY secret distribution
  static NoiseKPZ evalEncryptPk(const LocalParam &param);
  static NoiseKPZ evalAdd(const NoiseKPZ &lhs, const NoiseKPZ &rhs);
  static NoiseKPZ evalMultNoRelin(const LocalParam &resultParam,
                                  const NoiseKPZ &lhs, const NoiseKPZ &rhs);
  // l: number of digit
  // beta: base
  static NoiseKPZ evalModUp(const LocalParam &inputParam,
                            const NoiseKPZ &input);
  static NoiseKPZ evalRelinearizeBV(const LocalParam &inputParam,
                                    const NoiseKPZ &input);
  static NoiseKPZ evalModReduce(const LocalParam &inputParam,
                                const NoiseKPZ &input);
  // static NoiseKPZ evalRotate(const NoiseKPZ &input, double n, double t,
  // double std0, double numDigit, double beta);

  std::string toBound(const LocalParam &param) const;

  void print(llvm::raw_ostream &os) const { os << value; }

  std::string toString() const;

  friend llvm::raw_ostream &operator<<(llvm::raw_ostream &os,
                                       const NoiseKPZ &variance);

  friend Diagnostic &operator<<(Diagnostic &diagnostic,
                                const NoiseKPZ &variance);

 private:
  NoiseKPZType varianceType;
  std::optional<double> value;
};

}  // namespace heir
}  // namespace mlir

#endif  // INCLUDE_ANALYSIS_NOISEPROPAGATION_NOISEKPZ_H_
