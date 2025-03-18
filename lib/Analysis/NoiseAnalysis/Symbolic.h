#ifndef INCLUDE_ANALYSIS_NOISEANALYSIS_SYMBOLIC_H_
#define INCLUDE_ANALYSIS_NOISEANALYSIS_SYMBOLIC_H_

#include <cassert>
#include <map>
#include <string>
#include <vector>

#include "lib/Analysis/NoiseAnalysis/Noise.h"
#include "lib/Parameters/BGV/Params.h"

// #define IGNORE_SYMBOL

namespace mlir {
namespace heir {

enum SymbolType {
  // t * es
  EncryptSk,
  // t * (e_i * s + es * u_i)
  EncryptPk,
};

class Symbol {
 public:
  using ExponentType = int64_t;

  Symbol(const std::string &name, SymbolType type,
         ExponentType modReduceExponent = 0)
      : name(name), type(type) {}

  bool operator==(const Symbol &rhs) const {
    return name == rhs.name && type == rhs.type;
  };

  bool operator<(const Symbol &rhs) const;

  std::string getName() const { return name; }
  SymbolType getType() const { return type; }

 private:
  std::string name;
  SymbolType type;
};

class Expression {
 public:
  // care about double overflow...
  using CoefficientType = double;
  using FactorType = double;
  using ExponentType = int64_t;
  using SymbolsType = std::map<Symbol, ExponentType>;
  using ParamType = bgv::LocalParam;

  // to be deleted
  // Expression() = default;

  Expression(const Symbol &symbol) : name(symbol.getName()) {
    symbols[symbol] = 1;
  }

  bool operator==(const Expression &rhs) const {
    return name == rhs.name && symbols == rhs.symbols &&
           coefficient == rhs.coefficient;
  };

 private:
  Expression(const std::string &name, const SymbolsType &symbols,
             CoefficientType coefficient)
      : name(name), symbols(symbols), coefficient(coefficient) {}

  Expression(const std::string &name, SymbolsType &&symbols,
             CoefficientType coefficient)
      : name(name), symbols(symbols), coefficient(coefficient) {}

 public:
  // actually add is max/merge
  // TODO: support actual add
  Expression add(const Expression &rhs, ParamType resultParam) const;

  // Expression add(const Expression &rhs);
  Expression multiply(const Expression &rhs, ParamType resultParam) const;

  // scaled-noise part of mod reduce
  Expression modReduceScale(double modulus, ParamType resultParam) const;

  static SymbolsType mergeSymbols(const Expression::SymbolsType &lhs,
                                  const Expression::SymbolsType &rhs);
  static std::tuple<CoefficientType, std::vector<ExponentType>> computeFactor(
      SymbolsType symbols);

  double toVariance(ParamType param) const;

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

  SymbolsType getSymbols() const { return symbols; }

  // Expression itself may have a name
  std::string name;
  SymbolsType symbols;
  CoefficientType coefficient =
      1.0;  // Var[c * X], as usually mod reduce, record the inverse
};

}  // namespace heir
}  // namespace mlir

#endif  // INCLUDE_ANALYSIS_NOISEANALYSIS_SYMBOLIC_H_
