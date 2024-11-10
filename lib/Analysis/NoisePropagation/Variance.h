#ifndef INCLUDE_ANALYSIS_NOISEPROPAGATION_VARIANCE_H_
#define INCLUDE_ANALYSIS_NOISEPROPAGATION_VARIANCE_H_

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <optional>

#include "lib/Analysis/NoisePropagation/Params.h"
#include "llvm/include/llvm/Support/Debug.h"        // from @llvm-project
#include "llvm/include/llvm/Support/raw_ostream.h"  // from @llvm-project
#include "mlir/include/mlir/IR/Diagnostics.h"       // from @llvm-project
#include "mlir/include/mlir/IR/Value.h"             // from @llvm-project

#define DEBUG_TYPE "Variance"

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

  double logAlphaBound(int n) const { return log(alphaBound(n)) / log(2); }

  void print(llvm::raw_ostream &os) const { os << value; }

  std::string toString() const;

  std::string toBound(int n) const {
    if (varianceType == VarianceType::UNBOUNDED) {
      return "MAX";
    }
    std::stringstream stream;
    stream << std::fixed << std::setprecision(2) << logAlphaBound(n);
    return stream.str();
  }

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

  VarianceKey(Param p, int cv, int l, Value v, bool ghs)
      : p(p), cv(cv), l(l), v(v), ghs(ghs) {}

  std::string toDOTNode(
      const DenseMap<Value, std::string> &valueNameMap) const {
    return "\"" + valueNameMap.at(v) +
           //"_n_" + std::to_string(p.n) + "_dS_" +
           //  std::to_string(p.digitSize) + "_dN_" + std::to_string(p.dnum) +
           "_cv_" + std::to_string(cv) + "_l_" + std::to_string(l) + "\"";
  }

  void print(llvm::raw_ostream &os) const {
    os << "(n " << p.n << " dS " << p.digitSize << " dN " << p.dnum << " cv "
       << cv << " l " << l << " ghs " << int(ghs) << ")";
  }

  bool operator==(const VarianceKey &rhs) const {
    return p == rhs.p && cv == rhs.cv && l == rhs.l && v == rhs.v &&
           ghs == rhs.ghs;
  }

  bool operator!=(const VarianceKey &rhs) const { return !(*this == rhs); }

  bool operator<(const VarianceKey &rhs) const {
    if (p != rhs.p) {
      return p < rhs.p;
    }
    if (cv != rhs.cv) {
      return cv < rhs.cv;
    }
    if (l != rhs.l) {
      return l < rhs.l;
    }
    if (ghs != rhs.ghs) {
      return ghs < rhs.ghs;
    }
    return false;
  }

  bool dominatedBy(const VarianceKey &rhs) const {
    assert(p == rhs.p && v == rhs.v && ghs == rhs.ghs);
    return cv <= rhs.cv && l <= rhs.l && !(cv == rhs.cv && l == rhs.l);
  }

  bool sameParam(const VarianceKey &rhs) const {
    return p == rhs.p && v == rhs.v;
  }

  bool sameLevel(const VarianceKey &rhs) const {
    return p == rhs.p && l == rhs.l && ghs == rhs.ghs;
  }

  bool canModReduce() const { return l > 0; };
  bool canRelinearize() const {
    return cv > 2 && (p.maxRelinSkDeg == 0 || p.maxRelinSkDeg + 1 >= cv);
  };

  bool isFinal() const { return l == 0 && cv == 2; };

  bool isAbortFinal() const {
    return !isFinal() && cv > 2 &&
           (p.maxRelinSkDeg != 0 && p.maxRelinSkDeg + 1 < cv);
  };

  Param getParam() const { return p; };

  static VarianceKey evalModReduce(const VarianceKey &lhs) {
    assert(lhs.canModReduce());
    return VarianceKey(lhs.p, lhs.cv, lhs.l - 1, lhs.v, lhs.ghs);
  }

  VarianceKey evalModReduce() const {
    return VarianceKey::evalModReduce(*this);
  }

  static VarianceKey evalMultNoRelin(const VarianceKey &lhs,
                                     const VarianceKey &rhs, Value result) {
    assert(lhs.sameLevel(rhs));
    return VarianceKey(lhs.p, lhs.cv + rhs.cv - 1, lhs.l, result, lhs.ghs);
  }

  VarianceKey evalMultNoRelin(const VarianceKey &rhs, Value result) const {
    return VarianceKey::evalMultNoRelin(*this, rhs, result);
  }

  static VarianceKey evalRelinearizeBV(const VarianceKey &lhs) {
    assert(lhs.canRelinearize());
    return VarianceKey(lhs.p, lhs.cv - 1, lhs.l, lhs.v, lhs.ghs);
  }

  VarianceKey evalRelinearizeBV() const {
    return VarianceKey::evalRelinearizeBV(*this);
  }

  static VarianceKey evalRelinearizeGHSModUp(const VarianceKey &lhs) {
    assert(!lhs.ghs);
    return VarianceKey(lhs.p, lhs.cv, lhs.l, lhs.v, true);
  }

  VarianceKey evalRelinearizeGHSModUp() const {
    return VarianceKey::evalRelinearizeGHSModUp(*this);
  }

  static VarianceKey evalRelinearizeGHSModDown(const VarianceKey &lhs) {
    assert(lhs.ghs);
    return VarianceKey(lhs.p, lhs.cv, lhs.l, lhs.v, false);
  }

  VarianceKey evalRelinearizeGHSModDown() const {
    return VarianceKey::evalRelinearizeGHSModDown(*this);
  }

  Variance bound(const Variance &v) const {
    if (v.logAlphaBound(p.n) >= p.logQlP(l, ghs)) {
      return Variance::unbounded();
    }
    return v;
  }

  friend llvm::raw_ostream &operator<<(llvm::raw_ostream &os,
                                       const VarianceKey &key);

  // friend Diagnostic &operator<<(Diagnostic &diagnostic,
  //                               const VarianceKey &key);
 private:
  Param p;
  int cv;
  int l;
  Value v;
  bool ghs;
};

class VarianceValues {
 public:
  friend class VarianceStates;

  VarianceValues() = default;

  std::string toDOTNode(
      const DenseMap<Value, std::string> &valueNameMap) const {
    std::string str;
    str += k.toDOTNode(valueNameMap);
    // str += " [label=\"" + getVariance().toBound(k.p.n) + " " + getReason() +
    // "\"]";
    return str;
  }

  std::string toDOTEdge(
      const DenseMap<Value, std::string> &valueNameMap) const {
    std::string str;
    for (auto &p : v) {
      bool markBold = false;
      if (std::get<1>(p) == getParents()) {
        markBold = true;
      }
      for (auto &parent : std::get<1>(p)) {
        str += parent.toDOTNode(valueNameMap) + " -> " +
               k.toDOTNode(valueNameMap) + " [label=\"" +
               std::get<0>(p).toBound(k.p.n) + " " + std::get<2>(p) + "\"";
        if (markBold && std::get<0>(p).isBounded()) {
          str += " color=black fontcolor=black";
        } else {
          str += " color=gray fontcolor=gray";
          if (!std::get<0>(p).isBounded()) {
            str += " style=dashed";
          }
        }
        str += "]\n";
      }
    }
    return str;
  }

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

  bool operator==(const VarianceValues &rhs) const {
    return k == rhs.k && v == rhs.v;
  }

  bool operator!=(const VarianceValues &rhs) const { return !(*this == rhs); }

  VarianceValues(VarianceKey k, Variance var,
                 std::vector<VarianceKey> parents = {}, std::string reason = "")
      : k(k) {
    insert(std::make_tuple(var, parents, reason));
  }

  void insert(const std::tuple<Variance, std::vector<VarianceKey>, std::string>
                  &tuple) {
    v.push_back(tuple);
  }

  void insert(
      std::tuple<Variance, std::vector<VarianceKey>, std::string> &&tuple) {
    v.push_back(std::move(tuple));
  }

  void join(const VarianceValues &rhs) {
    assert(k == rhs.k);
    for (auto &p : rhs.v) {
      insert(p);
    }
  }

  void join(VarianceValues &&rhs) {
    assert(k == rhs.k);
    for (auto &p : rhs.v) {
      insert(std::move(p));
    }
  }

  std::tuple<Variance, std::vector<VarianceKey>, std::string> getMinimal()
      const {
    auto index = 0;
    for (size_t i = 0; i != v.size(); ++i) {
      Variance res = Variance::min(std::get<0>(v[index]), std::get<0>(v[i]));
      if (res == std::get<0>(v[i])) {
        index = i;
      }
    }
    return v[index];
  }

  Variance getVariance() const { return std::get<0>(getMinimal()); }

  std::vector<VarianceKey> getParents() const {
    return std::get<1>(getMinimal());
  }

  std::string getReason() const { return std::get<2>(getMinimal()); }

  bool reachable() const { return getVariance().isBounded(); }

  static VarianceValues evalEncryptPk(Value result, Param p) {
    int cv = 2;
    double std0 = 3.2;
    VarianceKey k = VarianceKey(p, cv, p.L, result, false);
    auto v = Variance::evalEncryptPk(k.p.n, k.p.t, std0);
    return VarianceValues(k, k.bound(v), {}, "enc");
  }

  static VarianceValues evalModReduce(const VarianceValues &lhs) {
    VarianceKey k = lhs.k.evalModReduce();
    Variance v = Variance::evalModReduce(lhs.getVariance(), 1L << k.p.qi[k.l],
                                         k.p.n, k.p.t);
    return VarianceValues(k, k.bound(v), {lhs.k}, "modd");
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
    return VarianceValues(k, k.bound(v), {lhs.k, rhs.k}, "mult");
  }

  VarianceValues evalMultNoRelin(const VarianceValues &rhs,
                                 Value result) const {
    return VarianceValues::evalMultNoRelin(*this, rhs, result);
  }

  static VarianceValues evalRelinearizeBV(const VarianceValues &lhs) {
    assert(lhs.k.canRelinearize());
    VarianceKey k = lhs.k.evalRelinearizeBV();
    Variance v =
        Variance::evalRelinearizeBV(lhs.getVariance(), k.p.n, k.p.t, 3.2,
                                    k.p.numDigit(k.l, k.ghs), k.p.digit());
#if 0
    LLVM_DEBUG(llvm::dbgs()
               << "original " << lhs.getVariance().toBound(k.p.n) << " relin "
               << v.toBound(k.p.n) << k.p.logQlP(k.l, k.ghs) << "\n");
#endif
    return VarianceValues(k, k.bound(v), {lhs.k}, "relin");
  }

  VarianceValues evalRelinearizeBV() const {
    return VarianceValues::evalRelinearizeBV(*this);
  }

  static VarianceValues evalRelinearizeGHS(const VarianceValues &lhs) {
    VarianceKey kModUp = lhs.k.evalRelinearizeGHSModUp();
    VarianceKey kRelin = kModUp.evalRelinearizeBV();
    VarianceKey kModDown = kRelin.evalRelinearizeGHSModDown();

    Variance vModUp = Variance::evalModUp(lhs.getVariance(), kModUp.p.P(),
                                          kModUp.p.n, kModUp.p.t);
    if (!kModUp.bound(vModUp).isBounded()) {
      return VarianceValues(kModDown, kModUp.bound(vModUp), {lhs.k}, "relin");
    }

    assert(kModUp.canRelinearize());
    Variance vRelin = Variance::evalRelinearizeBV(
        vModUp, kRelin.p.n, kRelin.p.t, 3.2,
        kRelin.p.numDigit(kRelin.l, kRelin.ghs), kRelin.p.digit());
    if (!kRelin.bound(vRelin).isBounded()) {
      return VarianceValues(kModDown, kRelin.bound(vRelin), {lhs.k}, "relin");
    }

    Variance vModDown = Variance::evalModReduce(vRelin, kModDown.p.P(),
                                                kModDown.p.n, kModDown.p.t);
#if 0
    LLVM_DEBUG(llvm::dbgs()
               << "original " << lhs.getVariance().toBound(kModUp.p.n)
               << " modup " << vModUp.toBound(kModUp.p.n) << " bound "
               << kModUp.p.logQlP(kModUp.l, kModUp.ghs) << " relin "
               << vRelin.toBound(kModUp.p.n) << " bound "
               << kRelin.p.logQlP(kRelin.l, kRelin.ghs) << " moddown "
               << vModDown.toBound(kModUp.p.n) << " bound "
               << kModDown.p.logQlP(kModDown.l, kModDown.ghs) << "\n");
#endif
    return VarianceValues(kModDown, kModDown.bound(vModDown), {lhs.k}, "relin");
  }

  VarianceValues evalRelinearizeGHS() const {
    return VarianceValues::evalRelinearizeGHS(*this);
  }

  VarianceValues evalRelinearize() const {
    if (k.p.dnum == 0) {
      return VarianceValues::evalRelinearizeBV(*this);
    } else {
      return VarianceValues::evalRelinearizeGHS(*this);
    }
  }

  friend llvm::raw_ostream &operator<<(llvm::raw_ostream &os,
                                       const VarianceValues &values);

  // friend Diagnostic &operator<<(Diagnostic &diagnostic,
  //                               const VarianceValues &values);

 private:
  VarianceKey k;
  // variance, its parent(s) and reason
  std::vector<std::tuple<Variance, std::vector<VarianceKey>, std::string>> v;
};

class VarianceStates {
 public:
  VarianceStates() = default;

  std::string toDOTNode(
      const DenseMap<Value, std::string> &valueNameMap) const {
    std::string str;
    auto &v = states.begin()->second.begin()->first.v;
    str += "subgraph cluster_" + valueNameMap.at(v) + "{\n";
    for (auto &[p, kToVs] : states) {
      for (auto &[k, vs] : kToVs) {
        if (!k.isAbortFinal() && vs.reachable()) {
          str += vs.toDOTNode(valueNameMap);
          str += "\n";
        }
      }
    }
    str += "}\n";
    return str;
  }

  std::string toDOTEdge(
      const DenseMap<Value, std::string> &valueNameMap) const {
    std::string str;
    for (auto &[p, kToVs] : states) {
      for (auto &[k, vs] : kToVs) {
        if (!k.isAbortFinal() && vs.reachable()) {
          str += vs.toDOTEdge(valueNameMap);
        }
      }
    }
    return str;
  }

  void print(llvm::raw_ostream &os) const {
    os << "\n[\n";
    for (auto &[p, kToVs] : states) {
      for (auto &[k, vs] : kToVs) {
        os << "\t";
        os << vs;
        os << ",\n";
      }
    }
    os << "]\n";
  }

  bool operator==(const VarianceStates &rhs) const {
    return states == rhs.states;
  }

  void insert(const VarianceValues &values) {
    if (!values.reachable()) {
      return;
    }
    auto &k = values.k;
    auto &p = values.k.p;
    if (states.find(p) != states.end()) {
      auto &kToVs = states[p];
      if (kToVs.find(k) != kToVs.end()) {
        auto &vs = kToVs[k];
        vs.join(values);
      } else {
        kToVs[k] = values;
      }
    } else {
      states[p] = {{k, values}};
    }
  }

  void insert(VarianceValues &&values) {
    if (!values.reachable()) {
      return;
    }
    auto &&k = values.k;
    auto &&p = values.k.p;
    if (states.find(p) != states.end()) {
      auto &kToVs = states[p];
      if (kToVs.find(k) != kToVs.end()) {
        auto &vs = kToVs[k];
        vs.join(std::move(values));
      } else {
        VarianceKey kc = values.k;
        kToVs[kc] = std::move(values);
      }
    } else {
      VarianceKey kc = values.k;
      Param pc = values.k.p;
      states[pc] = {{kc, std::move(values)}};
    }
  }

  // join to left side
  static VarianceStates &join(VarianceStates &lhs, const VarianceStates &rhs) {
    // special handling of those assert monotonicity
    if (lhs == rhs) {
      return lhs;
    }
    if (lhs.size() == 0) {
      lhs.states = rhs.states;
      return lhs;
    }
    for (auto &[p, kToVs] : rhs.states) {
      for (auto &[k, vs] : kToVs) {
        lhs.insert(vs);
      }
    }
    return lhs;
  }

  VarianceStates join(VarianceStates &rhs) {
    return VarianceStates::join(*this, rhs);
  }

  size_t size() const {
    size_t ret = 0;
    for (auto &[_, kToVs] : states) {
      ret += kToVs.size();
    }
    return ret;
  }

  // use modreduce / relin to expand the space
  void expand() {
    LLVM_DEBUG(llvm::dbgs() << "expand before: " << size() << "\n");
    for (auto &[p, kToVs] : states) {
      std::set<VarianceKey> visited;
      while (visited.size() != kToVs.size()) {
        for (auto &[k, vs] : kToVs) {
          if (visited.find(k) != visited.end()) {
            continue;
          }

          bool dominated = false;
          for (auto &[rk, rvs] : kToVs) {
            if (visited.find(rk) != visited.end()) {
              continue;
            }
            if (k.sameParam(rk) && k.dominatedBy(rk)) {
              dominated = true;
              break;
            }
          }

          if (dominated) {
            continue;
          }

          // then try to create new keys
          visited.insert(k);

          if (k.canRelinearize() && vs.reachable()) {
            insert(vs.evalRelinearize());
          }

          if (k.canModReduce() && vs.reachable()) {
            insert(vs.evalModReduce());
          }
        }
      }
    }
    LLVM_DEBUG(llvm::dbgs() << "expand after: " << size() << "\n");
  }

  std::vector<Param> reachable() const {
    std::vector<Param> ret;
    for (auto &[p, kToVs] : states) {
      for (auto &[k, vs] : kToVs) {
        if (k.isFinal() && vs.reachable()) {
          ret.push_back(p);
        }
      }
    }
    std::sort(ret.begin(), ret.end());
    return ret;
  }

  static VarianceStates evalEncryptPk(Value result, int t, int l) {
    VarianceStates vss;
    std::vector<Param> params;
#if 0
    params.push_back(Param::genParam(2, 0, 0, t, 60, 0));
#endif
#if 1
    for (auto depth : {l, l - 1}) {
      for (auto relinDeg : {0, 2, 3}) {
        for (auto qiSize : {0, 30, 40, 50, 60}) {
          // for (auto digitSize : {0, 30, 2}) {
          for (auto digitSize = 30; digitSize >= 2; digitSize--) {
            params.push_back(
                Param::genParam(depth, digitSize, 0, t, qiSize, relinDeg));
          }
          for (auto dnum : {2, 3, depth + 1}) {
            params.push_back(
                Param::genParam(depth, 0, dnum, t, qiSize, relinDeg));
          }
        }
      }
    }
#endif
    LLVM_DEBUG(llvm::dbgs() << "param size: " << params.size() << "\n");
    for (auto &p : params) {
#if 0
      LLVM_DEBUG(llvm::dbgs() << p << "\n");
#endif
      auto vs = VarianceValues::evalEncryptPk(result, p);
      vss.insert(std::move(vs));
    }
    vss.expand();
    return vss;
  }

  static VarianceStates evalMultNoRelin(const VarianceStates &lhs,
                                        const VarianceStates &rhs,
                                        Value result) {
    VarianceStates vss;
    for (auto &[p, lm] : lhs.states) {
      if (rhs.states.find(p) != rhs.states.end()) {
        auto &rm = rhs.states.at(p);
        for (auto &[lk, l] : lm) {
          for (auto &[rk, r] : rm) {
            if (lk.sameLevel(rk) && l.getVariance().isBounded() &&
                r.getVariance().isBounded()) {
              auto vs = l.evalMultNoRelin(r, result);
              vss.insert(std::move(vs));
            }
          }
        }
      }
    }
    vss.expand();
    return vss;
  }

  friend llvm::raw_ostream &operator<<(llvm::raw_ostream &os,
                                       const VarianceStates &variance);

  // friend Diagnostic &operator<<(Diagnostic &diagnostic,
  //                               const VarianceStates &variance);
 private:
  std::map<Param, std::map<VarianceKey, VarianceValues>> states;
};

}  // namespace heir
}  // namespace mlir

#endif  // INCLUDE_ANALYSIS_NOISEPROPAGATION_VARIANCE_H_
