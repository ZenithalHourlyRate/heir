#include "lib/Analysis/NoisePropagation/Params.h"

#define DEBUG_TYPE "Params"

namespace mlir {
namespace heir {

std::map<SchemeParamsFactory::SchemeParamKey, SchemeParam>
    SchemeParamsFactory::AllParams = {};

std::set<LocalParam> LocalParamFactory::AllLocalParams = {};

int SchemeParam::logQlP(int l, bool ghs) const {
  assert(l <= L);

  int ret = 0;
  for (size_t i = 0; i <= l; ++i) {
    ret += qi[i];
  }

  if (ghs) {
    for (int p : pi) {
      ret += p;
    }
  }

  return ret;
}

void SchemeParam::print(llvm::raw_ostream &os) const {
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

// TODO: fix this weak < operator
bool SchemeParam::operator<(const SchemeParam &rhs) const {
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

SchemeParam SchemeParamsFactory::genParam(int depth, int digitSize, int dnum,
                                          int64_t t, int qiSize,
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

    SchemeParam param;
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

    if (dnum == 0 && digitSize != 0) {
      assert(param.digitSize <= param.qi[0]);
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
  SchemeParam param;
  return param;
}

}  // namespace heir
}  // namespace mlir
