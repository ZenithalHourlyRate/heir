#ifndef INCLUDE_ANALYSIS_NOISEANALYSIS_SYMBOLIC_H_
#define INCLUDE_ANALYSIS_NOISEANALYSIS_SYMBOLIC_H_

#include <cassert>
#include <map>
#include <numeric>
#include <string>
#include <vector>

#include "lib/Parameters/BGV/Params.h"
#include "llvm/include/llvm/Support/Debug.h"  // from @llvm-project

// #define DEBUG_TYPE "Symbolic"

// #define IGNORE_SYMBOL

namespace mlir {
namespace heir {

class Symbol {
 public:
  Symbol(const std::string &name) : name(name) {}

  bool operator<(const Symbol &rhs) const { return name < rhs.name; }
  bool operator==(const Symbol &rhs) const { return name == rhs.name; }

  std::string getName() const { return name; }

 private:
  std::string name;
};

class Monomial {
 public:
  using ExponentType = int64_t;
  using SymbolsType = std::map<Symbol, ExponentType>;
  using CoefficientType = double;

  Monomial() = default;
  Monomial(const Symbol &symbol) { symbols[symbol] = 1; }

  Monomial(const Symbol &symbol, ExponentType exponent)
      : symbols({{symbol, exponent}}) {}

  Monomial(const SymbolsType &symbols) : symbols(symbols) {}

  bool operator<(const Monomial &rhs) const { return symbols < rhs.symbols; }
  bool operator==(const Monomial &rhs) const { return symbols == rhs.symbols; }

  SymbolsType getSymbols() const { return symbols; }

  static SymbolsType mergeSymbols(const SymbolsType &lhs,
                                  const SymbolsType &rhs) {
    SymbolsType newSymbols;
    auto updateSymbols = [&](const SymbolsType &symbols) {
      for (auto &[symbol, exponent] : symbols) {
        auto find = newSymbols.find(symbol);
        if (find != newSymbols.end()) {
          find->second += exponent;
        } else {
          newSymbols[symbol] = exponent;
        }
      }
    };
    updateSymbols(lhs);
    updateSymbols(rhs);
    return newSymbols;
  }

  static Monomial multiply(const Monomial &lhs, const Monomial &rhs) {
    auto newSymbols = mergeSymbols(lhs.symbols, rhs.symbols);
    return Monomial(newSymbols);
  }

  std::string toString() const {
    std::string ret;
    bool firstTime = true;
    for (auto &[symbol, exponent] : symbols) {
      if (!firstTime) {
        ret += " * ";
      }
      ret += symbol.getName();
      if (exponent != 1) {
        ret += "^" + std::to_string(exponent);
      }
      firstTime = false;
    }
    return ret;
  }

 private:
  SymbolsType symbols;
};

class Expression {
 public:
  using CoefficientType = double;
  using MonomialsType = std::map<Monomial, double>;

  Expression() = default;
  Expression(const Monomial &monomial) { monomials.insert({monomial, 1.0}); }

  Expression(const MonomialsType &monomials) : monomials(monomials) {}

  Expression(const MonomialsType &&monomials) : monomials(monomials) {}

  bool operator==(const Expression &rhs) const {
    return monomials == rhs.monomials;
  }

  static MonomialsType mergeMonomials(const MonomialsType &lhs,
                                      const MonomialsType &rhs) {
    MonomialsType newMonomials;
    auto updateMonomials = [&](const MonomialsType &monomials) {
      for (auto &[monomial, coefficient] : monomials) {
        auto find = newMonomials.find(monomial);
        if (find != newMonomials.end()) {
          find->second += coefficient;
        } else {
          newMonomials[monomial] = coefficient;
        }
      }
    };
    updateMonomials(lhs);
    updateMonomials(rhs);
    return newMonomials;
  }

  static Expression add(const Expression &lhs, const Expression &rhs) {
    auto newMonomials = mergeMonomials(lhs.monomials, rhs.monomials);
    return Expression(newMonomials);
  }

  static Expression multiply(const Expression &lhs, const Expression &rhs) {
    MonomialsType newMonomials;
    for (auto &[lhsMonomial, lhsCoefficient] : lhs.monomials) {
      for (auto &[rhsMonomial, rhsCoefficient] : rhs.monomials) {
        auto newMonomial = Monomial::multiply(lhsMonomial, rhsMonomial);
        auto newCoefficient = lhsCoefficient * rhsCoefficient;
        auto find = newMonomials.find(newMonomial);
        if (find != newMonomials.end()) {
          find->second += newCoefficient;
        } else {
          newMonomials[newMonomial] = newCoefficient;
        }
      }
    }
    return Expression(newMonomials);
  }

  static Expression multiply(const Expression &lhs,
                             CoefficientType coefficient) {
    MonomialsType newMonomials;
    for (auto &[lhsMonomial, lhsCoefficient] : lhs.monomials) {
      newMonomials[lhsMonomial] = lhsCoefficient * coefficient;
    }
    return Expression(newMonomials);
  }

  // This is variance expression!
  double toVariance(int ringDim) const {
    double variance = 0;
    for (auto &[monomial, coefficient] : monomials) {
      auto factor = coefficient * coefficient;
      auto exponentSum = 0;
      for (auto &[symbol, exponent] : monomial.getSymbols()) {
        // factorial(exponent)
        factor *= tgamma(exponent + 1);
        exponentSum += exponent;
        auto name = symbol.getName();
        if (name[0] == 'e') {
          factor *= pow(3.19 * 3.19, exponent);
        } else if (name[0] == 's' || name[0] == 'u') {
          factor *= pow(2.0 / 3, exponent);
        } else if (name[0] == 'k') {
          // [-1/2, 1/2]
          factor *= pow(1.0 / 12, exponent);
        }
      }
      factor *= pow(ringDim, exponentSum - 1);
      variance += factor;
    }
    return variance;
  }

  std::vector<int> getDegreeForGeneralizedNormal() const {
    double maxCoefficient = 0;
    std::vector<int> selectedDegreeVec;
    for (auto &[monomial, coefficient] : monomials) {
      std::vector<int> degreeVec;
      for (auto &[symbol, exponent] : monomial.getSymbols()) {
        degreeVec.push_back(exponent);
      }
      if (coefficient > maxCoefficient) {
        maxCoefficient = coefficient;
        selectedDegreeVec = degreeVec;
      }
    }
    return selectedDegreeVec;
  }

  std::string toString() const {
    std::string ret;
    bool firstTime = true;
    for (auto &[monomial, coefficient] : monomials) {
      if (!firstTime) {
        ret += " + ";
      }
      if (coefficient != 1.0) {
        ret += std::to_string(coefficient) + " * ";
      }
      ret += monomial.toString();
      firstTime = false;
    }
    return ret;
  }

  // for Lattice in DataFlowFramework

  static Expression join(const Expression &lhs, const Expression &rhs) {
    // prefer lhs when equal
    if (lhs.monomials.size() >= rhs.monomials.size()) {
      return lhs;
    }
    return rhs;
  }

  void print(raw_ostream &os) const { os << toString(); }

 private:
  MonomialsType monomials;
};

}  // namespace heir
}  // namespace mlir

#endif  // INCLUDE_ANALYSIS_NOISEANALYSIS_SYMBOLIC_H_
