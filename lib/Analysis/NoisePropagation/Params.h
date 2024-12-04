#ifndef INCLUDE_ANALYSIS_NOISEPROPAGATION_PARAMS_H_
#define INCLUDE_ANALYSIS_NOISEPROPAGATION_PARAMS_H_

#include <cmath>

#include "llvm/include/llvm/Support/Debug.h"        // from @llvm-project
#include "llvm/include/llvm/Support/raw_ostream.h"  // from @llvm-project
#include "mlir/include/mlir/IR/Diagnostics.h"       // from @llvm-project

namespace mlir {
namespace heir {

struct LWEParam {
  int n;
  int maxQ;
};

// tenary
static struct LWEParam HEStd_128_classic[] = {
    {1024, 27},   {2048, 54},   {4096, 109},   {8192, 218},
    {16384, 438}, {32768, 881}, {65536, 1747}, {131072, 3523}};

class SchemeParam {
 public:
  int n;
  int64_t t;
  int L;
  // qi.size() == L + 1
  std::vector<int> qi;

  int maxRelinSkDeg;

  // BV
  int digitSize;
  int digitPerQi;

  // GHS
  int dnum;
  int alpha;
  std::vector<int> pi;

  double std0 = 3.2;

 public:
  double P() const {
    double ret = 0.0;
    for (int p : pi) {
      ret += 1L << p;
    }
    return ret;
  }

  double digit() const {
    // if digitSize == 0 and dnum == 0, then digitSize = qi[0]
    if (digitSize == 0) {
      return pow(2.0, qi[0]);
    }
    return pow(2.0, digitSize);
  }

  double numDigit(int l, bool ghs) const {
    if (dnum == 0) {
      return digitPerQi * (l + 1);
    }
    int num = ceil(double(l + 1) / alpha);
    if (ghs) {
      num += 1;
    }
    return num;
  }

  int logQlP(int l, bool ghs) const;

  void print(llvm::raw_ostream &os) const;

  bool operator==(const SchemeParam &rhs) const {
    return n == rhs.n && t == rhs.t && digitSize == rhs.digitSize &&
           digitPerQi == rhs.digitPerQi && maxRelinSkDeg == rhs.maxRelinSkDeg &&
           L == rhs.L && dnum == rhs.dnum && alpha == rhs.alpha &&
           qi == rhs.qi && pi == rhs.pi;
  }

  bool operator!=(const SchemeParam &rhs) const { return !(*this == rhs); }

  bool operator<(const SchemeParam &rhs) const;

  friend llvm::raw_ostream &operator<<(llvm::raw_ostream &os,
                                       const SchemeParam &param) {
    param.print(os);
    return os;
  }
};

class SchemeParamsFactory {
 public:
  using SchemeParamKey = std::tuple<int, int, int, int, int, int>;

  static std::map<SchemeParamKey, SchemeParam> AllParams;

  static const SchemeParam *getSchemeParam(int depth, int digitSize, int dnum,
                                           int64_t t, int qiSize,
                                           int maxRelinSkDeg) {
    SchemeParamKey k(depth, digitSize, dnum, t, qiSize, maxRelinSkDeg);
    if (AllParams.find(k) == AllParams.end()) {
      auto p = genParam(depth, digitSize, dnum, t, qiSize, maxRelinSkDeg);
      AllParams[k] = std::move(p);
    }
    return &AllParams.at(k);
  }

  static SchemeParam genParam(int depth, int digitSize, int dnum, int64_t t,
                              int qiSize, int maxRelinSkDeg);

  static SchemeParam genConcreteParam(int depth, int digitSize, int dnum,
                                      int64_t t, const std::vector<int> &qiSize,
                                      int maxRelinSkDeg);
};

class LocalParam {
 public:
  friend class LocalParamFactory;

  LocalParam() = default;

  LocalParam(const SchemeParam *p, int cv, int l) : p(p), cv(cv), l(l) {}

  void print(llvm::raw_ostream &os) const {
    os << "(n " << p->n << " dS " << p->digitSize << " dN " << p->dnum << " cv "
       << cv << " l " << l << ")";
  }

  bool operator==(const LocalParam &rhs) const {
    return *p == *rhs.p && cv == rhs.cv && l == rhs.l;
  }

  bool operator!=(const LocalParam &rhs) const { return !(*this == rhs); }

  bool operator<(const LocalParam &rhs) const {
    if (*p != *rhs.p) {
      return *p < *rhs.p;
    }
    if (cv != rhs.cv) {
      return cv < rhs.cv;
    }
    if (l != rhs.l) {
      return l < rhs.l;
    }
    return false;
  }

  bool sameParam(const LocalParam &rhs) const { return *p == *rhs.p; }

  const SchemeParam *getSchemeParam() const { return p; };

  int getDimension() const { return cv; }
  int getLevel() const { return l; }

  friend llvm::raw_ostream &operator<<(llvm::raw_ostream &os,
                                       const LocalParam &localParam) {
    localParam.print(os);
    return os;
  }

  // friend Diagnostic &operator<<(Diagnostic &diagnostic,
  //                               const LocalParam &key);
 private:
  const SchemeParam *p;
  int cv;
  int l;
};

class LocalParamFactory {
  static std::set<LocalParam> AllLocalParams;

 public:
  static const LocalParam *getLocalParam(const SchemeParam *p, int cv, int l) {
    auto k = LocalParam(p, cv, l);
    auto res = AllLocalParams.insert(k);
    return &*res.first;
  }
};

}  // namespace heir
}  // namespace mlir

#endif  // INCLUDE_ANALYSIS_NOISEPROPAGATION_PARAMS_H_
