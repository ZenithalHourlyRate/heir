#ifndef INCLUDE_ANALYSIS_NOISEPROPAGATION_VARIANCE_H_
#define INCLUDE_ANALYSIS_NOISEPROPAGATION_VARIANCE_H_

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <optional>

#include "llvm/include/llvm/Support/raw_ostream.h"  // from @llvm-project
#include "mlir/include/mlir/IR/Diagnostics.h"       // from @llvm-project

namespace mlir {
namespace heir {

struct LatticeParam {
  int n;
  int maxQ;
};

// tenary
static struct LatticeParam HEStd_128_classic[] = {
    {1024, 27}, {2048, 54}, {4096, 109}, {8192, 218}, {16384, 438},
};

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

  static Variance min(const Variance &lhs, const Variance &rhs) {
    assert(lhs.varianceType == VarianceType::SET &&
           rhs.varianceType == VarianceType::SET);
    return Variance::of(std::min(lhs.getValue(), rhs.getValue()));
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
    // os << variance << "(" << n << " " << t << " " << cv << " " << l << ") "
    //    << "Bound(" << std::to_string(log(variance.alphaBound(n)) / log(2))
    //    << ")";
    os << "(" << cv << " " << l << "): " << "Bound("
       << std::to_string(log(variance.alphaBound(n)) / log(2)) << ")";
    os << "history(";
    for (auto str : history) {
      os << str;
      os << ", ";
    }
    os << ")";
  }

  bool sameState(const VarianceState &rhs) const {
    return n == rhs.n && t == rhs.t && cv == rhs.cv && l == rhs.l;
  }

  bool sameLevel(const VarianceState &rhs) const {
    return n == rhs.n && t == rhs.t && l == rhs.l;
  }

  VarianceState &inheritHistory(const VarianceState &rhs) {
    history.insert(history.end(), rhs.history.begin(), rhs.history.end());
    return *this;
  }

  VarianceState &mergeHistory(const VarianceState &lhs,
                              const VarianceState &rhs) {
    addHistory("(");
    inheritHistory(lhs);
    addHistory(")");
    addHistory("(");
    inheritHistory(rhs);
    addHistory(")");
    return *this;
  }

  VarianceState &addHistory(std::string str) {
    history.push_back(str);
    return *this;
  }

  bool operator==(const VarianceState &rhs) const {
    return sameState(rhs) && variance == rhs.variance;
  }

  static VarianceState join(const VarianceState &lhs,
                            const VarianceState &rhs) {
    assert(lhs.sameState(rhs));
    auto v = Variance::min(lhs.variance, rhs.variance);
    auto vs = VarianceState(lhs.n, lhs.t, lhs.cv, lhs.l, v);
    if (v == lhs.variance) {
      vs.inheritHistory(lhs);
    } else {
      vs.inheritHistory(rhs);
    }
    vs.addHistory("join");
    return vs;
  }

  VarianceState join(const VarianceState &rhs) const {
    return VarianceState::join(*this, rhs);
  }

  static VarianceState evalEncryptPk(int n, int t, int l) {
    int cv = 2;
    auto v = Variance::evalEncryptPk(n, t, 3.2);
    return VarianceState(n, t, cv, l, v).addHistory("enc");
  }

  static VarianceState evalMultNoRelin(const VarianceState &lhs,
                                       const VarianceState &rhs) {
    assert(lhs.sameLevel(rhs));
    auto v = Variance::evalMultNoRelin(lhs.variance, rhs.variance, lhs.n);
    auto vs = VarianceState(lhs.n, lhs.t, lhs.cv + rhs.cv - 1, lhs.l, v);
    vs.mergeHistory(lhs, rhs);
    vs.addHistory("mult");
    return vs;
  }

  static VarianceState evalRelinearizeBV(const VarianceState &lhs) {
    auto v = Variance::evalRelinearizeBV(lhs.variance, lhs.n, lhs.t, 3.2, lhs.l,
                                         35156991246337);
    auto vs = VarianceState(lhs.n, lhs.t, lhs.cv - 1, lhs.l, v);
    vs.inheritHistory(lhs);
    vs.addHistory("relin");
    return vs;
  }

  static VarianceState evalModReduce(const VarianceState &lhs) {
    auto v =
        Variance::evalModReduce(lhs.variance, 35156991246337, lhs.n, lhs.t);
    auto vs = VarianceState(lhs.n, lhs.t, lhs.cv, lhs.l - 1, v);
    vs.inheritHistory(lhs);
    vs.addHistory("modd");
    return vs;
  }

  bool canModReduce() { return l > 1; };
  bool canRelinearize() { return cv > 2; };

 private:
  int n;
  int t;
  int cv;
  int l;
  Variance variance;

  std::vector<std::string> history;
};

class VarianceStates {
 public:
  VarianceStates() = default;
  VarianceStates(std::vector<VarianceState> states) : states(states) {}

  void print(llvm::raw_ostream &os) const {
    os << "\n[\n";
    for (auto &s : states) {
      os << "\t";
      s.print(os);
      os << ",\n";
    }
    os << "]\n";
  }

  bool operator==(const VarianceStates &rhs) const {
    if (states.size() != rhs.states.size()) {
      return false;
    }
    for (auto &l : states) {
      bool found = false;
      for (auto &r : rhs.states) {
        if (l.sameState(r)) {
          found = true;
          if (!(l == r)) {
            return false;
          }
        }
      }
      if (!found) {
        return false;
      }
    }
    return true;
  }

  void insert(VarianceState vs) {
    bool inserted = false;
    for (auto &s : states) {
      if (vs.sameState(s)) {
        s = vs.join(s);
        inserted = true;
        break;
      }
    }
    if (!inserted) {
      states.push_back(vs);
    }
  }

  static VarianceStates join(const VarianceStates &lhs,
                             const VarianceStates &rhs) {
    VarianceStates res = lhs;
    for (auto &r : rhs.states) {
      res.insert(r);
    }
    return res;
  }

  VarianceStates join(const VarianceStates &rhs) const {
    return VarianceStates::join(*this, rhs);
  }

  static VarianceStates evalEncryptPk(int t, int l) {
    VarianceStates vss;
    for (auto n : {1024}) {
      auto vs = VarianceState::evalEncryptPk(n, t, l);
      vss.insert(vs);
      for (auto i = 0; i != l - 1; ++i) {
        vs = VarianceState::evalModReduce(vs);
        vss.insert(vs);
      }
    }
    return vss;
  }

  static VarianceStates evalMultNoRelin(const VarianceStates &lhs,
                                        const VarianceStates &rhs) {
    VarianceStates vss;
    for (auto &l : lhs.states) {
      for (auto &r : rhs.states) {
        if (l.sameLevel(r)) {
          vss.insert(VarianceState::evalMultNoRelin(l, r));
        }
      }
    }
    VarianceStates others;
    for (auto &vs : vss.states) {
      others.insert(VarianceState::evalRelinearizeBV(vs));
      if (vs.canModReduce()) {
        others.insert(VarianceState::evalModReduce(vs));
        others.insert(
            VarianceState::evalRelinearizeBV(VarianceState::evalModReduce(vs)));
      }
    }
    return vss.join(others);
  }

  friend llvm::raw_ostream &operator<<(llvm::raw_ostream &os,
                                       const VarianceStates &variance);

 private:
  std::vector<VarianceState> states;
};

}  // namespace heir
}  // namespace mlir

#endif  // INCLUDE_ANALYSIS_NOISEPROPAGATION_VARIANCE_H_
