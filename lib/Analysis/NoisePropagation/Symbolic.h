#ifndef INCLUDE_ANALYSIS_NOISEPROPAGATION_SYMBOLIC_H_
#define INCLUDE_ANALYSIS_NOISEPROPAGATION_SYMBOLIC_H_

#include <cassert>
#include <map>
#include <string>

#include "lib/Analysis/NoisePropagation/Params.h"

// #define IGNORE_SYMBOL

namespace mlir {
namespace heir {

enum SymbolType {
  // t * es
  EncryptSk,
  // t * (e_i * s + es * u_i)
  EncryptPk,
  // t * (sum to l ( D_j * eksk_j )) where D_j in (-D/2, D/2)
  // D is digit decomposed from a ^ k, where k is the multiplication count
  RelinearizeBV,
  // t_i * s ^ k, where t_i in (-t/2, t/2)
  ModReduce,
};

class VarianceKey;

class Symbol {
 public:
  using ExponentType = int64_t;

  Symbol(std::string name, SymbolType type, ExponentType modReduceExponent = 0,
         ExponentType relinearizeBVExponent = 0)
      : name(name),
        type(type),
        modReduceExponent(modReduceExponent),
        relinearizeBVExponent(relinearizeBVExponent) {}

  bool operator==(const Symbol &rhs) const {
    return name == rhs.name && type == rhs.type;
  };

  bool operator<(const Symbol &rhs) const;

  std::string getName() const { return name; }
  SymbolType getType() const { return type; }
  ExponentType getModReduceExponent() const {
    assert(type == SymbolType::ModReduce);
    return modReduceExponent;
  }
  ExponentType getRelinearizeBVExponent() const {
    assert(type == SymbolType::RelinearizeBV);
    return relinearizeBVExponent;
  }

 private:
  std::string name;
  SymbolType type;
  ExponentType modReduceExponent;      // only used for ModReduce
  ExponentType relinearizeBVExponent;  // only used for RelinearizeBV
};

class Expression {
 public:
  // care about double overflow...
  using CoefficientType = double;
  using FactorType = double;
  using ExponentType = int64_t;
  using SymbolsType = std::map<Symbol, ExponentType>;

  // to be deleted
  Expression() = default;

  Expression(Symbol symbol, const VarianceKey *key,
             const SymbolsType &inheritedSymbols = {},
             Symbol::ExponentType multiplyCount = 1)
      : name(symbol.getName()),
        inheritedSymbols(inheritedSymbols),
        multiplyCount(multiplyCount),
        key(key) {
    symbols[symbol] = 1;
    auto oldFactor = std::get<0>(computeFactor(inheritedSymbols));
    auto newFactor = std::get<0>(computeFactor(getAllSymbols()));
    factor = newFactor / oldFactor;
  }

  bool operator==(const Expression &rhs) const {
    return name == rhs.name && symbols == rhs.symbols &&
           inheritedSymbols == rhs.inheritedSymbols &&
           coefficient == rhs.coefficient && multiplyCount == rhs.multiplyCount;
  };

 private:
  Expression(std::string name, const SymbolsType &symbols,
             const SymbolsType &inheritedSymbols, CoefficientType coefficient,
             FactorType factor, Symbol::ExponentType multiplyCount,
             const VarianceKey *key)
      : name(name),
        symbols(symbols),
        inheritedSymbols(inheritedSymbols),
        coefficient(coefficient),
        factor(factor),
        multiplyCount(multiplyCount),
        key(key) {}

  Expression(std::string name, SymbolsType &&symbols,
             SymbolsType &&inheritedSymbols, CoefficientType coefficient,
             FactorType factor, Symbol::ExponentType multiplyCount,
             const VarianceKey *key)
      : name(name),
        symbols(symbols),
        inheritedSymbols(inheritedSymbols),
        coefficient(coefficient),
        factor(factor),
        multiplyCount(multiplyCount),
        key(key) {}

 public:
  // actually add is max/merge
  // TODO: support actual add
  Expression add(const Expression &rhs) const;

  // Expression add(const Expression &rhs);
  Expression multiply(const Expression &rhs, const VarianceKey *key) const;

  // scaled-noise part of mod reduce
  Expression modReduceScale(double modulus) const;

  static SymbolsType mergeSymbols(const Expression::SymbolsType &lhs,
                                  const Expression::SymbolsType &rhs);
  static std::tuple<CoefficientType, std::vector<ExponentType>> computeFactor(
      SymbolsType symbols);

  double toVariance() const;

  std::string toString() const;

  // helper on name
  std::string nameSelect(const Expression &rhs, bool selectedLhs) const {
    return "sel(" + (selectedLhs ? name : rhs.name) + "," +
           (selectedLhs ? rhs.name : name) + ")";
  }
  std::string nameMultiply(const Expression &rhs) const {
    return "mul(" + name + "," + rhs.name + ")";
  }
  std::string nameModReduceScaled() const { return "modds(" + name + ")"; }
  std::string nameModReduceAdded() const { return "modda(" + name + ")"; }
  std::string nameRelinearizeBVAdded() const {
    return "relinBVa(" + name + ")";
  }

  SymbolsType getSymbols() const { return symbols; }
  SymbolsType getAllSymbols() const {
    return mergeSymbols(symbols, inheritedSymbols);
  }

  Symbol::ExponentType getMultiplyCount() const { return multiplyCount; }

  // Expression itself may have a name
  std::string name;
  SymbolsType symbols;
  SymbolsType inheritedSymbols;  // added-noise will inherit the corelation
  CoefficientType coefficient =
      1.0;  // Var[c * X], as usually mod reduce, record the inverse
  FactorType factor = 1.0;                 // k * Var[X]
  Symbol::ExponentType multiplyCount = 1;  // a ^ k

  // corresponding to a variance key
  const VarianceKey *key;
};

}  // namespace heir
}  // namespace mlir

#endif  // INCLUDE_ANALYSIS_NOISEPROPAGATION_SYMBOLIC_H_
