#include "lib/Analysis/NoisePropagation/Params.h"

// FIXME: only include nbtheory.h from @openfhe
#include "src/pke/include/openfhe.h"  // from @openfhe

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
    int maxQP = p.maxQ;

    SchemeParam param;
    param.n = p.n;
    param.t = t;
    param.digitSize = digitSize;
    param.digitPerQi = 1;
    param.L = depth;
    param.maxRelinSkDeg = maxRelinSkDeg;
    param.qi = std::vector<int>(depth + 1, qiSize);

    auto logQ = qiSize * (depth + 1);

    if (dnum == 0 && digitSize != 0) {
      param.digitPerQi = ceil(double(qiSize) / param.digitSize);
    }

    auto log2 = [](double x) { return int(ceil(log(x) / log(2))); };

    param.dnum = dnum;
    if (dnum != 0) {
      param.alpha = ceil(double(depth + 1) / dnum);
      param.digitSize = qiSize * param.alpha;

      // select logP such that dnum * beta * expansionFactor * Berr / P <
      // expansionFactor * t (expansionFactor * Bkey) ** 2
      // namely, near the bound of multiplication
      auto Berr = param.std0 * 6;
      int logPmin = param.digitSize + log2(dnum) + log2(Berr) - log2(param.n);
      if (logPmin < 20) {
        logPmin = 20;
      }
      int logP = logPmin;

      // does not meet the security requirement
      if (logQ + logP > maxQP) {
        continue;
      }

      int pMaxWidth = 60;
      int pMinWidth = 20;
      int pNum = ceil(double(logP) / pMaxWidth);

      // allocate min to them
      for (size_t i = 0; i != pNum; ++i) {
        param.pi.push_back(pMinWidth < logP ? pMinWidth : logP);
        logP -= pMinWidth;
      }
      // allocate the rest
      for (size_t i = 0; i != pNum; ++i) {
        auto left = pMaxWidth - pMinWidth;
        if (left > logP) {
          left = logP;
        }
        param.pi[i] += left;
        logP -= left;
      }
    } else {
      // does not meet the security requirement
      if (logQ > maxQP) {
        continue;
      }
      param.alpha = 0;
    }
    return param;
  }
  assert(false && "failed to generate good param");
  SchemeParam param;
  return param;
}

SchemeParam SchemeParamsFactory::genConcreteParam(
    int depth, int digitSize, int dnum, int64_t t,
    const std::vector<int> &qiSize, int maxRelinSkDeg) {
  for (auto &p : HEStd_128_classic) {
    int maxQP = p.maxQ;

    SchemeParam param;
    param.n = p.n;
    param.t = t;
    param.digitSize = digitSize;
    param.digitPerQi = 1;
    param.L = depth;
    param.maxRelinSkDeg = maxRelinSkDeg;

    std::vector<int64_t> qiImpl;

    auto logQ = 0;
    auto qiSizeMax = 0;
    for (auto qi : qiSize) {
      if (qi < 20) {
        qi = 20;
      }
      while (qi < 60) {
        try {
          auto res = lbcrypto::FirstPrime<lbcrypto::NativeInteger>(qi, 2 * p.n);
          qiImpl.push_back(res.ConvertToInt());
          break;
        } catch (...) {
          qi += 1;
        }
      }
      if (qi >= 60) {
        assert(false && "failed to generate good qi");
      }
      param.qi.push_back(qi);
      logQ += qi;
      qiSizeMax = std::max(qiSizeMax, qi);
    }
    LLVM_DEBUG(llvm::dbgs() << "logQ: " << logQ << "\n");
    for (auto qi : qiImpl) {
      LLVM_DEBUG(llvm::dbgs() << "qiImpl: " << qi << "\n");
    }

    if (dnum == 0 && digitSize != 0) {
      param.digitPerQi = ceil(double(qiSizeMax) / param.digitSize);
    }

    auto log2 = [](double x) { return int(ceil(log(x) / log(2))); };

    param.dnum = dnum;
    if (dnum != 0) {
      param.alpha = ceil(double(depth + 1) / dnum);

      // get max digitSize
      auto maxDigitSize = 0;
      for (auto i = 0; i != dnum; ++i) {
        auto thisDigitSize = 0;
        for (auto j = 0; j != param.alpha; ++j) {
          auto idx = i * param.alpha + j;
          if (idx > param.qi.size()) {
            break;
          }
          thisDigitSize += param.qi[idx];
        }
        maxDigitSize = std::max(maxDigitSize, thisDigitSize);
      }
      param.digitSize = maxDigitSize;

      // select logP such that dnum * beta * expansionFactor * Berr / P <
      // expansionFactor * t (expansionFactor * Bkey) ** 2
      // namely, near the bound of multiplication
      auto Berr = param.std0 * 6;
      int logPmin = param.digitSize + log2(dnum) + log2(Berr) - log2(param.n);
      if (logPmin < 20) {
        logPmin = 20;
      }
      int logP = logPmin;
      LLVM_DEBUG(llvm::dbgs() << "logP: " << logP << "\n");

      // does not meet the security requirement
      if (logQ + logP > maxQP) {
        continue;
      }

      int pMaxWidth = 60;
      int pMinWidth = 20;
      int pNum = ceil(double(logP) / pMaxWidth);

      // allocate min to them
      for (size_t i = 0; i != pNum; ++i) {
        param.pi.push_back(pMinWidth < logP ? pMinWidth : logP);
        logP -= pMinWidth;
      }
      // allocate the rest
      for (size_t i = 0; i != pNum; ++i) {
        auto left = pMaxWidth - pMinWidth;
        if (left > logP) {
          left = logP;
        }
        param.pi[i] += left;
        logP -= left;
      }
    } else {
      // does not meet the security requirement
      if (logQ > maxQP) {
        continue;
      }
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
