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
  int digitSize;
  int L;
  // qi.size() == L + 1
  std::vector<int> qi;
  int digitPerQi;
  // derived param
  int dnum;
  int alpha;
  std::vector<int> pi;

 public:
  void print(llvm::raw_ostream &os) const {
    os << "Param: " << "n = " << n << ", L = " << L << ", qi = [";
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
           L == rhs.L && dnum == rhs.dnum && alpha == rhs.alpha &&
           qi == rhs.qi && pi == rhs.pi;
  }

  friend llvm::raw_ostream &operator<<(llvm::raw_ostream &os,
                                       const Param &param) {
    param.print(os);
    return os;
  }

  static Param genParam(int depth, int digitSize, int dnum, int t) {
    for (auto &p : HEStd_128_classic) {
      int maxQ = p.maxQ;
      if (dnum != 0) {
        maxQ = ceil(double(maxQ) * dnum / (dnum + 1));
      }
      int width = ceil(double(maxQ) / (depth + 1));
      // not wide enough
      if (width < 30 || width > 60) {
        continue;
      }

      Param param;
      param.n = p.n;
      param.t = t;
      param.digitSize = digitSize;
      param.digitPerQi = 1;
      param.L = depth;
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
    Param param;
    return param;
  }
};

}  // namespace heir
}  // namespace mlir

#endif  // INCLUDE_ANALYSIS_NOISEPROPAGATION_PARAMS_H_
