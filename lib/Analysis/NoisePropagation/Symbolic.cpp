#include "lib/Analysis/NoisePropagation/Symbolic.h"

#include <cmath>

#include "lib/Analysis/NoisePropagation/Variance.h"
#include "llvm/include/llvm/Support/Debug.h"        // from @llvm-project
#include "llvm/include/llvm/Support/raw_ostream.h"  // from @llvm-project

#define DEBUG_TYPE "Symbolic"

namespace mlir {
namespace heir {

bool Symbol::operator<(const Symbol &rhs) const {
  if (int(type) != int(rhs.type)) {
    return int(type) < int(rhs.type);
  }
  return name < rhs.name;
}

std::string Expression::toString() const {
  // auto ret = name + " = ";
  auto ret = std::to_string(log(coefficient) / log(2));
  auto dumpSymbols = [&](const SymbolsType symbols) {
    for (auto &[symbol, exponent] : symbols) {
      ret += " * ";
      ret += symbol.getName();
      if (exponent != 1) {
        ret += " ^ " + std::to_string(exponent);
      }
    }
  };
  dumpSymbols(symbols);
  ret += " inherited ";
  dumpSymbols(inheritedSymbols);
  ret += " factor " + std::to_string(log(factor) / log(2));
  return ret;
}

Expression::SymbolsType Expression::mergeSymbols(
    const Expression::SymbolsType &lhs, const Expression::SymbolsType &rhs) {
  SymbolsType newSymbols;
  auto updateSymbols = [&](const SymbolsType symbols) {
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

Expression Expression::multiply(const Expression &rhs,
                                const VarianceKey *newKey) const {
  std::string newName = nameMultiply(rhs);
  CoefficientType newCoefficient = coefficient * rhs.coefficient;

  auto newSymbols = mergeSymbols(symbols, rhs.symbols);
  auto newInheritedSymbols =
      mergeSymbols(inheritedSymbols, rhs.inheritedSymbols);

  auto oldFactors = std::get<0>(computeFactor(inheritedSymbols)) *
                    std::get<0>(computeFactor(rhs.inheritedSymbols));
  auto nowFactor =
      std::get<0>(computeFactor(mergeSymbols(newSymbols, newInheritedSymbols)));
  auto newFactor = nowFactor / oldFactors;

  return Expression(newName, std::move(newSymbols),
                    std::move(newInheritedSymbols), newCoefficient, newFactor,
                    newKey);
}

Expression Expression::modReduceScale(double modulus) const {
  std::string newName = nameModReduceScaled();
  CoefficientType newCoefficient = coefficient * modulus;
  return Expression(newName, symbols, inheritedSymbols, newCoefficient, factor,
                    key);
}

Expression Expression::add(const Expression &rhs) const {
  auto varianceLhs = toVariance();
  auto varianceRhs = rhs.toVariance();

  auto selectedLhs = varianceLhs >= varianceRhs;
  std::string newName = nameSelect(rhs, selectedLhs);

  auto selectedSymbols = selectedLhs ? symbols : rhs.symbols;
  auto selectedInheritedSymbols =
      selectedLhs ? inheritedSymbols : rhs.inheritedSymbols;
  auto selectedCoefficient = selectedLhs ? coefficient : rhs.coefficient;
  auto selectedFactor = selectedLhs ? factor : rhs.factor;
  auto selectedKey = selectedLhs ? key : rhs.key;

  return Expression(newName, selectedSymbols, selectedInheritedSymbols,
                    selectedCoefficient, selectedFactor, selectedKey);
}

static inline double factorial(int n) { return tgamma(n + 1); }

std::tuple<Expression::CoefficientType, std::vector<Expression::ExponentType>>
Expression::computeFactor(Expression::SymbolsType symbols) {
  double result = 1.0;
  ExponentType orderSum = 0;
  ExponentType skPkSum = 0;
  ExponentType ePkSum = 0;
  ExponentType skModRSum = 0;
  ExponentType tModRSum = 0;
  ExponentType DRelinSum = 0;
  ExponentType eRelinSum = 0;
  for (auto &[symbol, exponent] : symbols) {
    if (symbol.getType() == SymbolType::EncryptPk) {
      orderSum += 2 * exponent;  // each Pk with order two symbols
      skPkSum += exponent;
      ePkSum += exponent;
      // on e; delay s later
      result *= factorial(exponent);
    } else if (symbol.getType() == SymbolType::ModReduce) {
      orderSum += (1 + symbol.getModReduceExponent()) *
                  exponent;  // each ModReduce with order (1 + k) symbols
      skModRSum += symbol.getModReduceExponent() * exponent;
      tModRSum += exponent;
      // on t; delay s later
      result *= factorial(exponent);
    } else if (symbol.getType() == SymbolType::RelinearizeBV) {
      orderSum += 2 * exponent;
      DRelinSum += exponent;
      eRelinSum += exponent;
      // on D; delay eksk later
      // should we? seems not that dominant...
      result *= factorial(exponent);
    } else {
      assert(false && "unsupported symbol type");
    }
  }
#if 0
  LLVM_DEBUG(llvm::dbgs()
          << "orderSum: " << orderSum
          << " skPkSum: " << skPkSum
          << " ePkSum: " << ePkSum
          << " skModRSum: " << skModRSum
          << " tModRSum: " << tModRSum
          << " DRelinSum: " << DRelinSum
          << " eRelinSum: " << eRelinSum
          << "\n");
#endif
  // additional term for s in sk
  result *= factorial(skPkSum + skModRSum + 1) / (skModRSum + 1);
  // additional term for eksk in sk
  result *= factorial(eRelinSum + 1);
  std::vector<ExponentType> exponents = {
      orderSum, skPkSum, ePkSum, skModRSum, tModRSum, DRelinSum, eRelinSum,
  };
  return std::make_tuple(result, exponents);
}

double Expression::toVariance() const {
  double N = key->p->n;
  double t = key->p->t;
  double numDigit = key->p->numDigit(key->l, key->ghs);
  double beta = key->p->digit();
  double result = 1.0;

  auto [_, exponents] = computeFactor(symbols);
  auto orderSum = exponents[0];
  auto skPkSum = exponents[1];
  auto ePkSum = exponents[2];
  auto skModRSum = exponents[3];
  auto tModRSum = exponents[4];
  auto DRelinSum = exponents[5];
  auto eRelinSum = exponents[6];

  result *= factor;
  result *= pow(N, orderSum - 1);
  result *= pow(
      t, orderSum - skModRSum - tModRSum);  // no t before modreduce added noise
  result *= pow(2.0 / 3, skPkSum + skModRSum);     // Var[S]
  result *= pow(3.19 * 3.19, ePkSum + eRelinSum);  // Var[E]
  result *= pow(numDigit * (beta * beta) / 12.0,
                DRelinSum);                       // Var[D] of digitNum times
  result *= pow(double(t) * t / 12.0, tModRSum);  // Var[T]
  result /= coefficient * coefficient;
  return result;
}

}  // namespace heir
}  // namespace mlir
