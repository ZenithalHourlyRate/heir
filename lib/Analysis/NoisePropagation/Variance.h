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

int VariancePower(int cv);

struct VarianceMajorFactor {
  VarianceMajorFactor() = default;
  VarianceMajorFactor(std::string reason, int order)
      : reason(reason), order(order) {}

  bool correlate(const VarianceMajorFactor &rhs) const {
    return reason == rhs.reason && order != 0 && rhs.order != 0;
  }

  int mergeOrder(const VarianceMajorFactor &rhs) const {
    return order + rhs.order;
  }

  VarianceMajorFactor merge(const VarianceMajorFactor &rhs) const {
    assert(correlate(rhs));
    return VarianceMajorFactor(reason, mergeOrder(rhs));
  }

  double getPower(const VarianceMajorFactor &rhs) const {
    assert(correlate(rhs));
    // LLVM_DEBUG(llvm::dbgs() << "reason " << reason << " getPower: " <<
    // VariancePower(mergeOrder(rhs)) << " " << VariancePower(order) << " " <<
    // VariancePower(rhs.order) << "\n");
    return double(VariancePower(mergeOrder(rhs))) /
           (VariancePower(order) * VariancePower(rhs.order));
  }

  std::string toString() const {
    return "M(" + reason + " " + std::to_string(order) + ")";
  }

  std::string reason;
  int order;
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
  static Variance of(double value, std::string reason, int order) {
    return Variance(VarianceType::SET, value,
                    VarianceMajorFactor(reason, order));
  }
  static Variance of(double value, VarianceMajorFactor factor) {
    return Variance(VarianceType::SET, value, factor);
  }

  /// Create an integer value range lattice value.
  /// The default constructor must be equivalent to the "entry state" of the
  /// lattice, i.e., an uninitialized noise variance.
  Variance(VarianceType varianceType = VarianceType::UNINITIALIZED,
           std::optional<double> value = std::nullopt,
           VarianceMajorFactor factor = VarianceMajorFactor())
      : varianceType(varianceType), value(value), factor(factor) {}

  bool isKnown() const { return varianceType == VarianceType::SET; }

  bool isInitialized() const {
    return varianceType != VarianceType::UNINITIALIZED;
  }

  bool isBounded() const { return varianceType != VarianceType::UNBOUNDED; }

  const double &getValue() const {
    assert(isKnown());
    return *value;
  }

  const VarianceMajorFactor getFactor() const { return factor; }

  void setFactor(VarianceMajorFactor factor0) { factor = factor0; }

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
                                  double n, double t);
  // l: number of digit
  // beta: base
  static Variance evalModUp(const Variance &input, double modulus, double n,
                            double t, int cv);
  static Variance evalRelinearizeBV(const Variance &input, double n, double t,
                                    double std0, double numDigit, double beta);
  static Variance evalModReduce(const Variance &input, double modulus, double n,
                                double t, int cv);
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
    return stream.str() + " " + factor.toString();
  }

  friend llvm::raw_ostream &operator<<(llvm::raw_ostream &os,
                                       const Variance &variance);

  friend Diagnostic &operator<<(Diagnostic &diagnostic,
                                const Variance &variance);

 private:
  VarianceType varianceType;
  std::optional<double> value;
  VarianceMajorFactor factor;
};

class VarianceKey {
 public:
  friend class VarianceKeyFactory;
  friend class VarianceValues;
  friend class VarianceStates;

  VarianceKey() = default;

  VarianceKey(const Param *p, int cv, int l, bool ghs)
      : p(p), cv(cv), l(l), ghs(ghs) {}

  std::string toDOTNode(const std::string valueName) const {
    return "\"" + valueName +
           //"_n_" + std::to_string(p.n) + "_dS_" +
           //  std::to_string(p.digitSize) + "_dN_" +
           //  std::to_string(p.dnum) +
           "_cv_" + std::to_string(cv) + "_l_" + std::to_string(l) + "\"";
  }

  void print(llvm::raw_ostream &os) const {
    os << "(n " << p->n << " dS " << p->digitSize << " dN " << p->dnum << " cv "
       << cv << " l " << l << " ghs " << int(ghs) << ")";
  }

  bool operator==(const VarianceKey &rhs) const {
    return *p == *rhs.p && cv == rhs.cv && l == rhs.l && ghs == rhs.ghs;
  }

  bool operator!=(const VarianceKey &rhs) const { return !(*this == rhs); }

  bool operator<(const VarianceKey &rhs) const {
    if (*p != *rhs.p) {
      return *p < *rhs.p;
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
    assert(*p == *rhs.p && ghs == rhs.ghs);
    return cv <= rhs.cv && l <= rhs.l && !(cv == rhs.cv && l == rhs.l);
  }

  bool sameParam(const VarianceKey &rhs) const { return *p == *rhs.p; }

  bool sameLevel(const VarianceKey &rhs) const {
    return *p == *rhs.p && l == rhs.l && ghs == rhs.ghs;
  }

  bool canModReduce() const { return l > 0; };
  bool canRelinearize() const {
    return cv > 2 && (p->maxRelinSkDeg == 0 || p->maxRelinSkDeg + 1 >= cv);
  };

  bool isFinal() const { return l == 0 && cv == 2; };

  bool isAbortFinal() const {
    return !isFinal() && cv > 2 &&
           (p->maxRelinSkDeg != 0 && p->maxRelinSkDeg + 1 < cv);
  };

  const Param &getParam() const { return *p; };

  // const VarianceKey *evalModReduce() const {
  //   return VarianceKeyFactory::evalModReduce(*this);
  // }

  // const VarianceKey *evalMultNoRelin(const VarianceKey &rhs) const {
  //   return VarianceKeyFactory::evalMultNoRelin(*this, rhs);
  // }

  // const VarianceKey *evalRelinearizeBV() const {
  //   return VarianceKeyFactory::evalRelinearizeBV(*this);
  // }

  // const VarianceKey *evalRelinearizeGHSModUp() const {
  //   return VarianceKeyFactory::evalRelinearizeGHSModUp(*this);
  // }

  // const VarianceKey *evalRelinearizeGHSModDown() const {
  //   return VarianceKeyFactory::evalRelinearizeGHSModDown(*this);
  // }

  Variance bound(const Variance &v) const {
    // FIXME: either better estimation or tigher bound
    if (v.logAlphaBound(p->n) >= p->logQlP(l, ghs) - 1 - 5) {
      return Variance::unbounded();
    }
    return v;
  }

  std::string toBound(const Variance &v) const { return v.toBound(p->n); }

  friend llvm::raw_ostream &operator<<(llvm::raw_ostream &os,
                                       const VarianceKey &key);

  // friend Diagnostic &operator<<(Diagnostic &diagnostic,
  //                               const VarianceKey &key);
 private:
  const Param *p;
  int cv;
  int l;
  bool ghs;
};

class VarianceKeyFactory {
  static std::set<VarianceKey> AllVarianceKeys;

  static const VarianceKey *getVarianceKey(const Param *p, int cv, int l,
                                           bool ghs) {
    auto k = VarianceKey(p, cv, l, ghs);
    auto res = AllVarianceKeys.insert(std::move(k));
    return &*res.first;
  }

 public:
  static const VarianceKey *evalEncryptPk(const Param *p) {
    int cv = 2;
    return getVarianceKey(p, cv, p->L, false);
  }

  static const VarianceKey *evalModReduce(const VarianceKey &lhs) {
    assert(lhs.canModReduce());
    return getVarianceKey(lhs.p, lhs.cv, lhs.l - 1, lhs.ghs);
  }

  static const VarianceKey *evalMultNoRelin(const VarianceKey &lhs,
                                            const VarianceKey &rhs) {
    assert(lhs.sameLevel(rhs));
    return getVarianceKey(lhs.p, lhs.cv + rhs.cv - 1, lhs.l, lhs.ghs);
  }

  static const VarianceKey *evalRelinearizeBV(const VarianceKey &lhs) {
    assert(lhs.canRelinearize());
    return getVarianceKey(lhs.p, lhs.cv - 1, lhs.l, lhs.ghs);
  }

  static const VarianceKey *evalRelinearizeGHSModUp(const VarianceKey &lhs) {
    assert(!lhs.ghs);
    return getVarianceKey(lhs.p, lhs.cv, lhs.l, true);
  }

  static const VarianceKey *evalRelinearizeGHSModDown(const VarianceKey &lhs) {
    assert(lhs.ghs);
    return getVarianceKey(lhs.p, lhs.cv, lhs.l, false);
  }
};

class VarianceStates;

enum VarianceParentType { Self, Operand0, Operand1 };

struct VarianceParent {
  VarianceParent() = default;

  VarianceParent(VarianceParentType type, const VarianceKey *parentKey,
                 size_t index, CostModel::Cost cost)
      : type(type), parentKey(parentKey), index(index), cost(cost) {}

  bool operator==(const VarianceParent &rhs) const {
    return type == rhs.type && *parentKey == *rhs.parentKey &&
           index == rhs.index && cost == rhs.cost;
  }

  VarianceParentType getType() const { return type; }

  const VarianceKey *getParentKey() const { return parentKey; }

  VarianceParentType type;
  const VarianceKey *parentKey;
  size_t index;
  CostModel::Cost cost;
};

struct VarianceParents {
  VarianceParents() = default;

  VarianceParents(std::vector<VarianceParent> parents, std::string reason,
                  CostModel::Cost cost)
      : parents(parents), reason(reason), cost(cost) {}

  bool operator==(const VarianceParents &rhs) const {
    return parents == rhs.parents && reason == rhs.reason;
  }

  const std::vector<VarianceParent> &getParents() const { return parents; }

  const std::string getReason() const { return reason; }

  const CostModel::Cost getCost() const { return cost; }

  std::vector<VarianceParent> parents;
  std::string reason;
  CostModel::Cost cost;
};

class VarianceValues {
 public:
  friend class VarianceStates;

  VarianceValues() = default;

  VarianceValues(const VarianceKey *k) : k(k) {}

  VarianceValues(const VarianceKey *k, Variance var, VarianceParents parents)
      : k(k) {
    insert(std::make_tuple(var, parents));
  }

  std::string toDOTNode(const std::string &valueName) const {
    std::string str;
    str += k->toDOTNode(valueName);
    // str += " [label=\"" + getVariance().toBound(k.p.n) + " " + getReason() +
    // "\"]";
    return str;
  }

  std::string toDOTEdge(
      std::vector<Value> values,
      const DenseMap<Value, std::string> &valueNameMap,
      const std::vector<std::tuple<VarianceKey, VarianceParents>> &selected)
      const;

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
    // os << "Bound(";
    // os << std::to_string(log(getVariance().alphaBound(k->p->n)) / log(2));
    // os << ") parent:(";
    // for (auto &parent : getParents()) {
    //   os << parent;
    //   os << ", ";
    // }
    // os << "); ";
  }

  bool operator==(const VarianceValues &rhs) const {
    return *k == *rhs.k && v == rhs.v;
  }

  bool operator!=(const VarianceValues &rhs) const { return !(*this == rhs); }

  void insert(const std::tuple<Variance, VarianceParents> &tuple) {
    if (!std::get<0>(tuple).isBounded()) {
      return;
    }
    v.push_back(tuple);
  }

  void insert(std::tuple<Variance, VarianceParents> &&tuple) {
    if (!std::get<0>(tuple).isBounded()) {
      return;
    }
    v.push_back(std::move(tuple));
  }

  void join(const VarianceValues &rhs) {
    assert(*k == *rhs.k);
    for (auto &p : rhs.v) {
      insert(p);
    }
  }

  void join(VarianceValues &&rhs) {
    assert(*k == *rhs.k);
    for (auto &p : rhs.v) {
      insert(std::move(p));
    }
  }

  const Variance &getVariance(size_t index) const {
    return std::get<0>(v[index]);
  }

  const VarianceParents &getParents(size_t index) const {
    return std::get<1>(v[index]);
  }

  const std::string getReason(size_t index) const {
    return getParents(index).getReason();
  }

  const CostModel::Cost getCost(size_t index) const {
    return getParents(index).getCost();
  }

  const size_t getMinimalByVariance() const {
    auto index = 0;
    for (size_t i = 0; i != v.size(); ++i) {
      Variance res = Variance::min(getVariance(index), getVariance(i));
      if (res == getVariance(i)) {
        index = i;
      }
    }
    return index;
  }

  const size_t getMinimalByCost() const {
#ifndef IGNORE_MODEL
    auto index = 0;
    for (size_t i = 0; i != v.size(); ++i) {
      CostModel::Cost res = std::min(getCost(index), getCost(i));
      if (res == getCost(i)) {
        index = i;
      }
    }
    return index;
#else
    return getMinimalByVariance();
#endif
  }

  const size_t getIndexByParents(const VarianceParents &currentParent) const {
    for (size_t i = 0; i != v.size(); ++i) {
      if (currentParent == getParents(i)) {
        return i;
      }
    }
    assert(false);
    return -1;
  }

  const Variance &getVarianceByMinVariance() const {
    return getVariance(getMinimalByVariance());
  }
  const Variance &getVarianceByMinCost() const {
    return getVariance(getMinimalByCost());
  }

  const Variance &getVarianceByParents(
      const VarianceParents &currentParent) const {
    return getVariance(getIndexByParents(currentParent));
  }

  const VarianceParents &getParentsByMinCost() const {
    return getParents(getMinimalByCost());
  }

  const VarianceParents &getParentsBySuccessorParent(
      const VarianceParent &successorParent) const {
    return getParents(successorParent.index);
  }

  const CostModel::Cost getCost() const { return getCost(getMinimalByCost()); }

  const std::vector<CostModel::Cost> getCosts() const {
    std::vector<CostModel::Cost> ret;
    for (size_t i = 0; i != v.size(); ++i) {
      ret.push_back(getCost(i));
    }
    return ret;
  }

  bool reachable() const {
    if (v.size() == 0) {
      return false;
    }
    return getVarianceByMinVariance().isBounded();
  }

  static VarianceValues evalEncryptPk(const Param *p) {
    double std0 = 3.2;
    auto *k = VarianceKeyFactory::evalEncryptPk(p);
    auto v = Variance::evalEncryptPk(k->p->n, k->p->t, std0);
    // TODO: encrypt cost?
    auto parents = VarianceParents({}, "enc", 0);
    return VarianceValues(k, k->bound(v), parents);
  }

  static VarianceValues evalModReduce(const VarianceValues &lhs) {
    const VarianceKey *k = VarianceKeyFactory::evalModReduce(*lhs.k);
    VarianceValues ret(k);
    for (size_t i = 0; i != lhs.v.size(); ++i) {
      Variance v = Variance::evalModReduce(
          lhs.getVariance(i), 1L << k->p->qi[k->l], k->p->n, k->p->t, k->cv);

      auto cost = lhs.getCost(i) + CostModel::getBGVModReduceCost(
                                       lhs.k->p->n, lhs.k->l, lhs.k->cv);
      auto parent =
          VarianceParent(VarianceParentType::Self, lhs.k, i, lhs.getCost(i));
      auto parents = VarianceParents({parent}, "modd", cost);
      ret.join(VarianceValues(k, k->bound(v), parents));
    }
    return ret;
  }

  VarianceValues evalModReduce() const {
    return VarianceValues::evalModReduce(*this);
  }

  static VarianceValues evalMultNoRelin(const VarianceValues &lhs,
                                        const VarianceValues &rhs) {
    assert(lhs.k->sameLevel(*rhs.k));
    const VarianceKey *k = VarianceKeyFactory::evalMultNoRelin(*lhs.k, *rhs.k);
    VarianceValues ret(k);
    for (size_t i = 0; i != lhs.v.size(); ++i) {
      for (size_t j = 0; j != rhs.v.size(); ++j) {
        Variance v = Variance::evalMultNoRelin(
            lhs.getVariance(i), rhs.getVariance(j), k->p->n, k->p->t);
        auto cost = lhs.getCost(i) + rhs.getCost(j) +
                    CostModel::getBGVMultCost(lhs.k->p->n, lhs.k->l, lhs.k->cv,
                                              rhs.k->cv);
        auto parentL = VarianceParent(VarianceParentType::Operand0, lhs.k, i,
                                      lhs.getCost(i));
        auto parentR = VarianceParent(VarianceParentType::Operand1, rhs.k, j,
                                      rhs.getCost(j));
#if 0
        LLVM_DEBUG(llvm::dbgs() << "n " << lhs.k->p->n << " l " << lhs.k->l << " left " << i << " cv " << lhs.k->cv << " right " << j << " cv " << rhs.k->cv << " lcost " << int(lhs.getCost(i)) << " rcost " << int(rhs.getCost(j)) << " cost " << int(cost) << "\n");
#endif
        auto parents = VarianceParents({parentL, parentR}, "mult", cost);
        ret.join(VarianceValues(k, k->bound(v), parents));
      }
    }
    return ret;
  }

  static VarianceValues evalRelinearizeBV(const VarianceValues &lhs) {
    assert(lhs.k->canRelinearize());
    auto *k = VarianceKeyFactory::evalRelinearizeBV(*lhs.k);
    VarianceValues ret(k);
    for (size_t i = 0; i != lhs.v.size(); ++i) {
      Variance v = Variance::evalRelinearizeBV(
          lhs.getVariance(i), k->p->n, k->p->t, 3.2,
          k->p->numDigit(k->l, k->ghs), k->p->digit());
      auto cost = lhs.getCost(i) +
                  CostModel::getBGVRelinBVCost(lhs.k->p->n, lhs.k->l, lhs.k->cv,
                                               lhs.k->p->digitSize);
#if 0
      LLVM_DEBUG(llvm::dbgs()
                 << "original " << lhs.getVariance().toBound(k.p->n) << " relin "
                 << v.toBound(k.p->n) << k.p->logQlP(k.l, k.ghs) << "\n");
#endif
      auto parent =
          VarianceParent(VarianceParentType::Self, lhs.k, i, lhs.getCost(i));
      auto parents = VarianceParents({parent}, "relin", cost);
      ret.join(VarianceValues(k, k->bound(v), parents));
    }
    return ret;
  }

  VarianceValues evalRelinearizeBV() const {
    return VarianceValues::evalRelinearizeBV(*this);
  }

  static VarianceValues evalRelinearizeGHS(const VarianceValues &lhs) {
    const VarianceKey *kModUp =
        VarianceKeyFactory::evalRelinearizeGHSModUp(*lhs.k);
    const VarianceKey *kRelin = VarianceKeyFactory::evalRelinearizeBV(*kModUp);
    const VarianceKey *kModDown =
        VarianceKeyFactory::evalRelinearizeGHSModDown(*kRelin);

    VarianceValues ret(kModDown);
    for (size_t i = 0; i != lhs.v.size(); ++i) {
      Variance vModUp =
          Variance::evalModUp(lhs.getVariance(i), kModUp->p->P(), kModUp->p->n,
                              kModUp->p->t, kModUp->cv);
      if (!kModUp->bound(vModUp).isBounded()) {
        continue;
      }

      assert(kModUp->canRelinearize());
      Variance vRelin = Variance::evalRelinearizeBV(
          vModUp, kRelin->p->n, kRelin->p->t, 3.2,
          kRelin->p->numDigit(kRelin->l, kRelin->ghs), kRelin->p->digit());
      if (!kRelin->bound(vRelin).isBounded()) {
        continue;
      }

      Variance vModDown =
          Variance::evalModReduce(vRelin, kModDown->p->P(), kModDown->p->n,
                                  kModDown->p->t, kModDown->cv);
#if 0
      LLVM_DEBUG(llvm::dbgs()
                 << "original " << lhs.getVariance().toBound(kModUp->p.n)
                 << " modup " << vModUp.toBound(kModUp->p.n) << " bound "
                 << kModUp->p.logQlP(kModUp->l, kModUp->ghs) << " relin "
                 << vRelin.toBound(kModUp->p.n) << " bound "
                 << kRelin->p.logQlP(kRelin->l, kRelin->ghs) << " moddown "
                 << vModDown.toBound(kModUp->p.n) << " bound "
                 << kModDown->p.logQlP(kModDown->l, kModDown->ghs) << "\n");
#endif
      auto cost = lhs.getCost(i) + CostModel::getBGVRelinHYBRIDCost(
                                       lhs.k->p->n, lhs.k->p->L, lhs.k->cv,
                                       lhs.k->l, lhs.k->p->dnum);
      auto parent =
          VarianceParent(VarianceParentType::Self, lhs.k, i, lhs.getCost(i));
      auto parents = VarianceParents({parent}, "relinG", cost);
      ret.join(VarianceValues(kModDown, kModDown->bound(vModDown), parents));
    }
    return ret;
  }

  VarianceValues evalRelinearizeGHS() const {
    return VarianceValues::evalRelinearizeGHS(*this);
  }

  VarianceValues evalRelinearize() const {
    if (k->p->dnum == 0) {
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
  const VarianceKey *k;
  // variance, its parent(s) and reason
  std::vector<std::tuple<Variance, VarianceParents>> v;
};

class VarianceStates {
 public:
  VarianceStates() = default;

  std::string toDOTNode(
      std::vector<Value> values,
      const DenseMap<Value, std::string> &valueNameMap,
      const std::vector<std::tuple<VarianceKey, VarianceParents>> &selected)
      const {
    std::string str;
    str +=
        std::string("subgraph cluster_") + valueNameMap.at(values[0]) + "{\n";
    for (auto &[p, kToVs] : states) {
      for (auto &[k, vs] : kToVs) {
        if (!k.isAbortFinal() && vs.reachable()) {
          bool sel = false;
          for (auto &[sk, _] : selected) {
            if (sk == k) {
              sel = true;
            }
          }
          if (!sel) {
            continue;
          }
          str += vs.toDOTNode(valueNameMap.at(values[0]));
          str += "\n";
        }
      }
    }
    str += "}\n";
    return str;
  }

  std::string toDOTEdge(
      std::vector<Value> values,
      const DenseMap<Value, std::string> &valueNameMap,
      const std::vector<std::tuple<VarianceKey, VarianceParents>> &selected)
      const {
    std::string str;
    for (auto &[p, kToVs] : states) {
      for (auto &[k, vs] : kToVs) {
        if (!k.isAbortFinal() && vs.reachable()) {
          str += vs.toDOTEdge(values, valueNameMap, selected);
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
    auto &k = *values.k;
    auto &p = *values.k->p;
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
    auto &k = *values.k;
    auto &p = *values.k->p;
    if (states.find(p) != states.end()) {
      auto &kToVs = states[p];
      if (kToVs.find(k) != kToVs.end()) {
        auto &vs = kToVs[k];
        vs.join(std::move(values));
      } else {
        VarianceKey kc = *values.k;
        kToVs[kc] = std::move(values);
      }
    } else {
      VarianceKey kc = *values.k;
      Param pc = *values.k->p;
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
    // LLVM_DEBUG(llvm::dbgs() << "expand before: " << size() << "\n");
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
    // LLVM_DEBUG(llvm::dbgs() << "expand after: " << size() << "\n");
  }

  std::vector<std::pair<Param, std::vector<CostModel::Cost>>> reachable()
      const {
    std::vector<std::pair<Param, std::vector<CostModel::Cost>>> ret;
    for (auto &[p, kToVs] : states) {
      for (auto &[k, vs] : kToVs) {
        if (k.isFinal() && vs.reachable()) {
          ret.push_back({p, vs.getCosts()});
        }
      }
    }
    std::sort(ret.begin(), ret.end());
    return ret;
  }

  VarianceKey getMinimalCostKey() const {
    VarianceKey key;
    CostModel::Cost cost = 1e20;
    for (auto &[p, kToVs] : states) {
      for (auto &[k, vs] : kToVs) {
        if (k.isFinal() && vs.reachable()) {
          if (vs.getCost() < cost) {
            key = k;
            cost = vs.getCost();
          }
        }
      }
    }
    return key;
  }

  const VarianceParents &getParentsByMinCost(VarianceKey key) const {
    auto &kToVs = states.find(key.getParam())->second;
    return kToVs.find(key)->second.getParentsByMinCost();
  }

  const VarianceParents &getParentsBySuccessorParent(
      VarianceKey key, const VarianceParent &successorParent) const {
    auto &kToVs = states.find(key.getParam())->second;
    return kToVs.find(key)->second.getParentsBySuccessorParent(successorParent);
  }

  Variance getVarianceByCurrentParents(
      VarianceKey key, const VarianceParents &currentParents) const {
    auto &kToVs = states.find(key.getParam())->second;
    return kToVs.find(key)->second.getVarianceByParents(currentParents);
  }

  static VarianceStates evalEncryptPk(int t, int l) {
    VarianceStates vss;

    std::vector<const Param *> params;
#if 0
    // params.push_back(ParamsFactory::getParam(2, 30, 0, t, 55, 2));
    // params.push_back(ParamsFactory::getParam(2, 2, 0, t, 55, 2));
    // params.push_back(ParamsFactory::getParam(2, 0, 2, t, 30, 2));
    // params.push_back(ParamsFactory::getParam(1, 30, 0, t, 55, 2));
    // params.push_back(ParamsFactory::getParam(1, 2, 0, t, 55, 2));
    // params.push_back(ParamsFactory::getParam(1, 0, 2, t, 30, 2));
    params.push_back(ParamsFactory::getParam(3, 30, 0, t, 55, 2));
    // params.push_back(ParamsFactory::getParam(3, 2, 0, t, 55, 2));
    // params.push_back(ParamsFactory::getParam(3, 0, 2, t, 55, 2));
#endif
#if 1
    for (auto depth : {l, l - 1}) {
      for (auto relinDeg : {2}) {
        for (auto qiSize : {45, 50, 53}) {
          for (auto digitSize : {30}) {
            params.push_back(ParamsFactory::getParam(depth, digitSize, 0, t,
                                                     qiSize, relinDeg));
          }
          for (auto dnum : {2}) {
            params.push_back(
                ParamsFactory::getParam(depth, 0, dnum, t, qiSize, relinDeg));
          }
        }
      }
    }
#endif
    // LLVM_DEBUG(llvm::dbgs() << "param size: " << params.size() << "\n");
    for (auto &p : params) {
#if 0
      LLVM_DEBUG(llvm::dbgs() << p << "\n");
#endif
      auto vs = VarianceValues::evalEncryptPk(p);
      vss.insert(std::move(vs));
    }
    vss.expand();
    return vss;
  }

  static VarianceStates evalMultNoRelin(const VarianceStates &lhs,
                                        const VarianceStates &rhs) {
    VarianceStates vss;
    for (auto &[p, lm] : lhs.states) {
      if (rhs.states.find(p) != rhs.states.end()) {
        auto &rm = rhs.states.at(p);
        for (auto &[lk, l] : lm) {
          for (auto &[rk, r] : rm) {
            if (lk.sameLevel(rk) && l.reachable() && r.reachable()) {
              auto vs = VarianceValues::evalMultNoRelin(l, r);
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
