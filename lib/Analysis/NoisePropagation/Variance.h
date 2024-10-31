#ifndef INCLUDE_ANALYSIS_NOISEPROPAGATION_VARIANCE_H_
#define INCLUDE_ANALYSIS_NOISEPROPAGATION_VARIANCE_H_

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <optional>

#include "lib/Analysis/NoisePropagation/Params.h"
#include "llvm/include/llvm/Support/raw_ostream.h"  // from @llvm-project
#include "mlir/include/mlir/IR/Diagnostics.h"       // from @llvm-project
#include "mlir/include/mlir/IR/Value.h"             // from @llvm-project

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

class VarianceKey {
 public:
  friend class VarianceValues;
  friend class VarianceStates;

  VarianceKey() = default;

  VarianceKey(Param p, int cv, int l, Value v) : p(p), cv(cv), l(l), v(v) {}

  void print(llvm::raw_ostream &os) const {
    os << "(n " << p.n << " dS " << p.digitSize << " dN " << p.dnum << " cv "
       << cv << " l " << l << ")";
  }

  bool operator==(const VarianceKey &rhs) const {
    return p == rhs.p && cv == rhs.cv && l == rhs.l && v == rhs.v;
  }

  bool sameLevel(const VarianceKey &rhs) const {
    return p == rhs.p && l == rhs.l;
  }

  bool canModReduce() const { return l > 1; };
  bool canRelinearize() const { return cv > 2; };

  static VarianceKey evalModReduce(const VarianceKey &lhs) {
    assert(lhs.canModReduce());
    return VarianceKey(lhs.p, lhs.cv, lhs.l - 1, lhs.v);
  }

  VarianceKey evalModReduce() const {
    return VarianceKey::evalModReduce(*this);
  }

  static VarianceKey evalMultNoRelin(const VarianceKey &lhs,
                                     const VarianceKey &rhs, Value result) {
    assert(lhs.sameLevel(rhs));
    return VarianceKey(lhs.p, lhs.cv + rhs.cv - 1, lhs.l, result);
  }

  VarianceKey evalMultNoRelin(const VarianceKey &rhs, Value result) const {
    return VarianceKey::evalMultNoRelin(*this, rhs, result);
  }

  static VarianceKey evalRelinearizeBV(const VarianceKey &lhs) {
    assert(lhs.canRelinearize());
    return VarianceKey(lhs.p, lhs.cv - 1, lhs.l, lhs.v);
  }

  VarianceKey evalRelinearizeBV() const {
    return VarianceKey::evalRelinearizeBV(*this);
  }

  friend llvm::raw_ostream &operator<<(llvm::raw_ostream &os,
                                       const VarianceKey &key);

 private:
  Param p;
  int cv;
  int l;
  Value v;
};

class VarianceValues {
  friend class VarianceStates;

  VarianceValues() = default;

  void print(llvm::raw_ostream &os) const {
    os << k;
    // for (auto &p : v) {
    //   os << "Bound(";
    //   os << std::to_string(log(p.first.alphaBound(k.p.n)) / log(2));
    //   os << ") parent:(";
    //   for (auto &parent : p.second) {
    //     os << parent;
    //     os << ", ";
    //   }
    //   os << "); ";
    // }
    os << "Bound(";
    os << std::to_string(log(getVariance().alphaBound(k.p.n)) / log(2));
    os << ") parent:(";
    for (auto &parent : getParents()) {
      os << parent;
      os << ", ";
    }
    os << "); ";
  }

  bool operator==(const VarianceValues &rhs) const { return v == rhs.v; }

  VarianceValues(VarianceKey k, Variance var,
                 std::vector<VarianceKey> parents = {})
      : k(k) {
    insert(std::make_pair(var, parents));
  }

  void insert(const std::pair<Variance, std::vector<VarianceKey>> &pair) {
    v.push_back(pair);
  }

  void join(const VarianceValues &rhs) {
    assert(k == rhs.k);
    for (auto &p : rhs.v) {
      insert(p);
    }
  }

  Variance getVariance() const {
    Variance res = v[0].first;
    for (auto &p : v) {
      res = Variance::min(res, p.first);
    }
    return res;
  }

  std::vector<VarianceKey> getParents() const {
    Variance res = v[0].first;
    std::vector<VarianceKey> parents = v[0].second;
    for (auto &p : v) {
      res = Variance::min(res, p.first);
      if (res == p.first) {
        parents = p.second;
      }
    }
    return parents;
  }

  static VarianceValues evalEncryptPk(Value result, Param p) {
    int cv = 2;
    double std0 = 3.2;
    VarianceKey k = VarianceKey(p, cv, p.L, result);
    auto v = Variance::evalEncryptPk(k.p.n, k.p.t, std0);
    return VarianceValues(k, v);
  }

  static VarianceValues evalModReduce(const VarianceValues &lhs) {
    VarianceKey k = lhs.k.evalModReduce();
    Variance v = Variance::evalModReduce(lhs.getVariance(), 1L << k.p.qi[k.l],
                                         k.p.n, k.p.t);
    return VarianceValues(k, v, {lhs.k});
  }

  VarianceValues evalModReduce() const {
    return VarianceValues::evalModReduce(*this);
  }

  static VarianceValues evalMultNoRelin(const VarianceValues &lhs,
                                        const VarianceValues &rhs,
                                        Value result) {
    assert(lhs.k.sameLevel(rhs.k));
    VarianceKey k = lhs.k.evalMultNoRelin(rhs.k, result);
    Variance v =
        Variance::evalMultNoRelin(lhs.getVariance(), rhs.getVariance(), k.p.n);
    return VarianceValues(k, v, {lhs.k, rhs.k});
  }

  VarianceValues evalMultNoRelin(const VarianceValues &rhs,
                                 Value result) const {
    return VarianceValues::evalMultNoRelin(*this, rhs, result);
  }

  static VarianceValues evalRelinearizeBV(const VarianceValues &lhs) {
    assert(lhs.k.canRelinearize());
    VarianceKey k = lhs.k.evalRelinearizeBV();
    Variance v = Variance::evalRelinearizeBV(lhs.getVariance(), k.p.n, k.p.t,
                                             3.2, k.l, 1L << k.p.digitSize);
    return VarianceValues(k, v, {lhs.k});
  }

  VarianceValues evalRelinearizeBV() const {
    return VarianceValues::evalRelinearizeBV(*this);
  }

  friend llvm::raw_ostream &operator<<(llvm::raw_ostream &os,
                                       const VarianceValues &values);

 private:
  VarianceKey k;
  // variance and its parent(s)
  std::vector<std::pair<Variance, std::vector<VarianceKey>>> v;
};

class VarianceStates {
 public:
  VarianceStates() = default;

  void print(llvm::raw_ostream &os) const {
    os << "\n[\n";
    for (auto &s : states) {
      os << "\t";
      os << s;
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
        if (l.k == r.k) {
          found = true;
          if (!(l.v == r.v)) {
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

  void insert(const VarianceValues &values) {
    bool inserted = false;
    for (auto &v : states) {
      if (v.k == values.k) {
        v.join(values);
        inserted = true;
        break;
      }
    }
    if (!inserted) {
      states.push_back(values);
    }
  }

  void insert(const VarianceKey &k, const Variance &v,
              const std::vector<VarianceKey> &parents = {}) {
    insert(VarianceValues(k, v, parents));
  }

  // join to left side
  static VarianceStates &join(VarianceStates &lhs, const VarianceStates &rhs) {
    for (auto &r : rhs.states) {
      lhs.insert(r);
    }
    return lhs;
  }

  VarianceStates join(const VarianceStates &rhs) {
    return VarianceStates::join(*this, rhs);
  }

  static VarianceStates evalEncryptPk(Value result, int t, int l) {
    VarianceStates vss;
    std::vector<Param> params = {Param::genParam(l, 30, 0, t),
                                 Param::genParam(l, 0, 0, t)};
    for (auto &p : params) {
      auto vs = VarianceValues::evalEncryptPk(result, p);
      vss.insert(vs);

      for (auto i = 0; i != l - 1; ++i) {
        vs = vs.evalModReduce();
        vss.insert(vs);
      }
    }
    return vss;
  }

  static VarianceStates evalMultNoRelin(const VarianceStates &lhs,
                                        const VarianceStates &rhs,
                                        Value result) {
    VarianceStates vss;
    for (auto &l : lhs.states) {
      for (auto &r : rhs.states) {
        if (l.k.sameLevel(r.k)) {
          auto vs = l.evalMultNoRelin(r, result);
          vss.insert(vs);
        }
      }
    }
    VarianceStates others;
    for (auto &vs : vss.states) {
      // all relin
      VarianceValues relin = vs;
      do {
        relin = relin.evalRelinearizeBV();
        others.insert(relin);
      } while (relin.k.canRelinearize());

      // all mod reduce ( + relin)
      // TODO: relin + mod reduce + relin
      if (vs.k.canModReduce()) {
        auto modd = vs.evalModReduce();
        others.insert(modd);

        VarianceValues relin = modd;
        do {
          relin = relin.evalRelinearizeBV();
          others.insert(relin);
        } while (relin.k.canRelinearize());
      }
    }
    return vss.join(others);
  }

  friend llvm::raw_ostream &operator<<(llvm::raw_ostream &os,
                                       const VarianceStates &variance);

 private:
  std::vector<VarianceValues> states;
};

}  // namespace heir
}  // namespace mlir

#endif  // INCLUDE_ANALYSIS_NOISEPROPAGATION_VARIANCE_H_
