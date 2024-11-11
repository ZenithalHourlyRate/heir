#ifndef INCLUDE_ANALYSIS_NOISEPROPAGATION_PARAMS_H_
#define INCLUDE_ANALYSIS_NOISEPROPAGATION_PARAMS_H_

#include <cmath>

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

class Param {
 public:
  int n;
  int t;
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

 public:
  double P() const {
    double ret = 0.0;
    for (size_t i = 0; i != pi.size(); ++i) {
      ret += 1L << pi[i];
    }
    return ret;
  }

  double digit() const { return pow(2.0, digitSize); }

  double numDigit(int l, bool ghs) const {
    if (dnum == 0) {
      return digitPerQi * (l + 1);
    } else {
      int num = ceil(double(l + 1) / alpha);
      if (ghs) {
        num += 1;
      }
      return num;
    }
  }

  int logQlP(int l, bool ghs) const {
    assert(l <= L);

    int ret = 0;
    for (size_t i = 0; i <= l; ++i) {
      ret += qi[i];
    }

    if (ghs) {
      for (size_t i = 0; i != pi.size(); ++i) {
        ret += pi[i];
      }
    }

    return ret;
  }

  void print(llvm::raw_ostream &os) const {
    os << "Param: " << "n = " << n << ", L = " << L
       << ", relinDeg = " << maxRelinSkDeg << ", qi = [";
    for (auto &q : qi) {
      os << q;
      os << ", ";
    }
    os << "]" << ", digitSize = " << digitSize << ", dnum = " << dnum
       << ", alpha = " << alpha << ", pi = [";
    for (auto &p : pi) {
      os << p;
      os << ", ";
    }
    os << "]";
  }

  bool operator==(const Param &rhs) const {
    return n == rhs.n && t == rhs.t && digitSize == rhs.digitSize &&
           digitPerQi == rhs.digitPerQi && maxRelinSkDeg == rhs.maxRelinSkDeg &&
           L == rhs.L && dnum == rhs.dnum && alpha == rhs.alpha &&
           qi == rhs.qi && pi == rhs.pi;
  }

  bool operator!=(const Param &rhs) const { return !(*this == rhs); }

  // TODO: fix this weak < operator
  bool operator<(const Param &rhs) const {
    if (n != rhs.n) {
      return n < rhs.n;
    }
    if (t != rhs.t) {
      return t < rhs.t;
    }
    if (L != rhs.L) {
      return L < rhs.L;
    }
    if (maxRelinSkDeg != rhs.maxRelinSkDeg) {
      if (maxRelinSkDeg == 0) {
        return false;
      }
      if (rhs.maxRelinSkDeg == 0) {
        return true;
      }
      return maxRelinSkDeg < rhs.maxRelinSkDeg;
    }
    if (digitSize != rhs.digitSize) {
      return digitSize < rhs.digitSize;
    }
    if (dnum != rhs.dnum) {
      return dnum < rhs.dnum;
    }
    if (logQlP(L, dnum != 0) != rhs.logQlP(rhs.L, rhs.dnum != 0)) {
      return logQlP(L, dnum != 0) < rhs.logQlP(rhs.L, rhs.dnum != 0);
    }
    if (qi[0] != rhs.qi[0]) {
      return qi[0] < rhs.qi[0];
    }
    return false;
  }

  friend llvm::raw_ostream &operator<<(llvm::raw_ostream &os,
                                       const Param &param) {
    param.print(os);
    return os;
  }
};

class ParamsFactory {
 public:
  using ParamKey = std::tuple<int, int, int, int, int, int>;

  static std::map<ParamKey, Param> AllParams;

  static const Param *getParam(int depth, int digitSize, int dnum, int t,
                               int qiSize, int maxRelinSkDeg) {
    ParamKey k(depth, digitSize, dnum, t, qiSize, maxRelinSkDeg);
    if (AllParams.find(k) == AllParams.end()) {
      auto p = genParam(depth, digitSize, dnum, t, qiSize, maxRelinSkDeg);
      AllParams[k] = std::move(p);
    }
    return &AllParams.at(k);
  }

  static Param genParam(int depth, int digitSize, int dnum, int t, int qiSize,
                        int maxRelinSkDeg) {
    for (auto &p : HEStd_128_classic) {
      int maxQ = p.maxQ;
      if (dnum != 0) {
        maxQ = ceil(double(maxQ) * dnum / (dnum + 1));
      }
      int maxWidth = ceil(double(maxQ) / (depth + 1));
      int width = maxWidth;
      if (qiSize == 0) {
        // not wide enough
        if (maxWidth < 20 || maxWidth > 60) {
          continue;
        }
      } else if (qiSize <= maxWidth) {
        width = qiSize;
      } else {
        continue;
      }

      Param param;
      param.n = p.n;
      param.t = t;
      param.digitSize = digitSize;
      param.digitPerQi = 1;
      param.L = depth;
      param.maxRelinSkDeg = maxRelinSkDeg;
      // TODO: support firstModSize
      int budget = maxQ;
      for (size_t i = 0; i != depth + 1; ++i) {
        param.qi.push_back(width < budget ? width : budget);
        budget -= width;
      }

      if (digitSize == 0) {
        param.digitSize = param.qi[0];
      }

      if (dnum == 0) {
        param.digitPerQi = ceil(double(param.qi[0]) / param.digitSize);
      }

      param.dnum = dnum;
      if (dnum != 0) {
        param.alpha = ceil(double(depth + 1) / dnum);
        param.digitSize = 0;
        for (size_t i = 0; i != param.alpha; ++i) {
          param.digitSize += param.qi[i];
        }

        // int logPmax = p.maxQ - maxQ;
        int logPmin = param.qi[0] * param.alpha;
        // assert(abs(logPmin - logPmax) < 3);
        //  TODO: select a proper logP
        int logP = logPmin;
        int pMaxWidth = 60;
        int pNum = ceil(double(logP) / pMaxWidth);

        for (size_t i = 0; i != pNum; ++i) {
          param.pi.push_back(pMaxWidth < logP ? pMaxWidth : logP);
          logP -= pMaxWidth;
        }
      } else {
        param.alpha = 0;
      }
      return param;
    }
    assert(false && "failed to generate good param");
    Param param;
    return param;
  }
};

}  // namespace heir
}  // namespace mlir

#endif  // INCLUDE_ANALYSIS_NOISEPROPAGATION_PARAMS_H_
