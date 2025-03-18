#include "lib/Analysis/NoiseAnalysis/Symbolic.h"

#include <cmath>

#include "lib/Analysis/NoiseAnalysis/Noise.h"
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
  auto [factor, _] = computeFactor(symbols);
  ret += " factor " + std::to_string(factor);
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
                                ParamType resultParam) const {
  std::string newName = nameMultiply(rhs);
  CoefficientType newCoefficient = coefficient * rhs.coefficient;
  auto newSymbols = mergeSymbols(symbols, rhs.symbols);

  return Expression(newName, std::move(newSymbols), newCoefficient);
}

Expression Expression::modReduceScale(double modulus,
                                      ParamType resultParam) const {
  std::string newName = nameModReduceScaled();
  CoefficientType newCoefficient = coefficient * modulus;
  return Expression(newName, symbols, newCoefficient);
}

Expression Expression::add(const Expression &rhs, ParamType resultParam) const {
  // Note: this is not how things....
  auto varianceLhs = toVariance(resultParam);
  auto varianceRhs = rhs.toVariance(resultParam);

  auto selectedLhs = varianceLhs >= varianceRhs;
  std::string newName = nameSelect(rhs, selectedLhs);

  auto selectedSymbols = selectedLhs ? symbols : rhs.symbols;
  auto selectedCoefficient = selectedLhs ? coefficient : rhs.coefficient;

  return Expression(newName, selectedSymbols, selectedCoefficient);
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
  ExponentType DRelinASum = 0;
  for (auto &[symbol, exponent] : symbols) {
    if (symbol.getType() == SymbolType::EncryptPk) {
      orderSum += 2 * exponent;  // each Pk with order two symbols
      skPkSum += exponent;
      ePkSum += exponent;
      // on e; delay s later
      result *= factorial(exponent);
      //} else if (symbol.getType() == SymbolType::ModReduce) {
      //  orderSum += (1 + symbol.getModReduceExponent()) *
      //              exponent;  // each ModReduce with order (1 + k) symbols
      //  skModRSum += symbol.getModReduceExponent() * exponent;
      //  tModRSum += exponent;
      //  // on t; delay s later
      //  result *= factorial(exponent);
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
  // correction term for a in D in relin added error
  result *= factorial(DRelinASum);
  // additional term for eksk in sk
  result *= factorial(eRelinSum + 1);
  std::vector<ExponentType> exponents = {
      orderSum, skPkSum,   ePkSum,    skModRSum,
      tModRSum, DRelinSum, eRelinSum, DRelinASum,
  };
  return std::make_tuple(result, exponents);
}

double Expression::toVariance(ParamType param) const {
  double N = param.getSchemeParam()->getRingDim();
  double t = param.getSchemeParam()->getPlaintextModulus();
  // double numDigit = key->p->numDigit(key->l, key->ghs);
  // double beta = key->p->digit();
  double result = 1.0;

  auto [factor, exponents] = computeFactor(symbols);
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
  // result *= pow(numDigit * (beta * beta) / 12.0,
  //               DRelinSum);                       // Var[D] of digitNum times
  result *= pow(double(t) * t / 12.0, tModRSum);  // Var[T]
  result /= coefficient * coefficient;
  return result;
}

}  // namespace heir
}  // namespace mlir
