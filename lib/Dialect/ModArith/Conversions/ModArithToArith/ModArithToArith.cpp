#include "lib/Dialect/ModArith/Conversions/ModArithToArith/ModArithToArith.h"

#include "lib/Dialect/ModArith/IR/ModArithOps.h"
#include "lib/Dialect/ModArith/IR/ModArithTypes.h"
#include "lib/Utils/ConversionUtils/ConversionUtils.h"
#include "mlir/include/mlir/IR/ImplicitLocOpBuilder.h"  // from @llvm-project
#include "mlir/include/mlir/IR/MLIRContext.h"           // from @llvm-project
#include "mlir/include/mlir/IR/TypeUtilities.h"         // from @llvm-project
#include "mlir/include/mlir/Transforms/DialectConversion.h"  // from @llvm-project
#include "mlir/include/mlir/Transforms/GreedyPatternRewriteDriver.h"  // from @llvm-project

namespace mlir {
namespace heir {
namespace mod_arith {

#define GEN_PASS_DEF_MODARITHTOARITH
#include "lib/Dialect/ModArith/Conversions/ModArithToArith/ModArithToArith.h.inc"

IntegerType convertModArithType(ModArithType type) {
  APInt modulus = type.getModulus().getValue();
  return IntegerType::get(type.getContext(), modulus.getBitWidth());
}

Type convertModArithLikeType(ShapedType type) {
  if (auto modArithType = llvm::dyn_cast<ModArithType>(type.getElementType())) {
    return type.cloneWith(type.getShape(), convertModArithType(modArithType));
  }
  return type;
}

class ModArithToArithTypeConverter : public TypeConverter {
 public:
  ModArithToArithTypeConverter(MLIRContext *ctx) {
    addConversion([](Type type) { return type; });
    addConversion(
        [](ModArithType type) -> Type { return convertModArithType(type); });
    addConversion(
        [](ShapedType type) -> Type { return convertModArithLikeType(type); });
  }
};

// A herlper function to generate the attribute or type
// needed to represent the result of modarith op as an integer
// before applying a remainder operation
template <typename Op>
TypedAttr modulusAttr(Op op, bool mul = false,
                      std::optional<APInt> constant = std::nullopt) {
  auto type = op.getResult().getType();
  auto modArithType = getResultModArithType(op);
  APInt modulus = modArithType.getModulus().getValue();

  auto width = modulus.getBitWidth();
  if (mul) {
    width *= 2;
  }

  auto intType = IntegerType::get(op.getContext(), width);
  APInt intValue = constant == std::nullopt ? modulus : *constant;
  auto truncmod = intValue.zextOrTrunc(width);

  if (auto st = mlir::dyn_cast<ShapedType>(type)) {
    auto containerType = st.cloneWith(st.getShape(), intType);
    return DenseElementsAttr::get(containerType, truncmod);
  }
  return IntegerAttr::get(intType, truncmod);
}

// used for extui/trunci
template <typename Op>
inline Type modulusType(Op op, bool mul = false) {
  return modulusAttr(op, mul).getType();
}

struct ConvertEncapsulate : public OpConversionPattern<EncapsulateOp> {
  ConvertEncapsulate(mlir::MLIRContext *context)
      : OpConversionPattern<EncapsulateOp>(context) {}

  using OpConversionPattern::OpConversionPattern;

  LogicalResult matchAndRewrite(
      EncapsulateOp op, OpAdaptor adaptor,
      ConversionPatternRewriter &rewriter) const override {
    rewriter.replaceAllUsesWith(op.getResult(), adaptor.getOperands()[0]);
    rewriter.eraseOp(op);
    return success();
  }
};

struct ConvertExtract : public OpConversionPattern<ExtractOp> {
  ConvertExtract(mlir::MLIRContext *context)
      : OpConversionPattern<ExtractOp>(context) {}

  using OpConversionPattern::OpConversionPattern;

  LogicalResult matchAndRewrite(
      ExtractOp op, OpAdaptor adaptor,
      ConversionPatternRewriter &rewriter) const override {
    rewriter.replaceAllUsesWith(op.getResult(), adaptor.getOperands()[0]);
    rewriter.eraseOp(op);
    return success();
  }
};

struct ConvertReduce : public OpConversionPattern<ReduceOp> {
  ConvertReduce(mlir::MLIRContext *context)
      : OpConversionPattern<ReduceOp>(context) {}

  using OpConversionPattern::OpConversionPattern;

  LogicalResult matchAndRewrite(
      ReduceOp op, OpAdaptor adaptor,
      ConversionPatternRewriter &rewriter) const override {
    ImplicitLocOpBuilder b(op.getLoc(), rewriter);

    auto cmod = b.create<arith::ConstantOp>(modulusAttr(op));
    // ModArithType ensures cmod can be correctly interpreted as a signed number
    auto rems = b.create<arith::RemSIOp>(adaptor.getOperands()[0], cmod);
    auto add = b.create<arith::AddIOp>(rems, cmod);
    // TODO(#710): better with a subifge
    auto remu = b.create<arith::RemUIOp>(add, cmod);
    rewriter.replaceOp(op, remu);
    return success();
  }
};

// It is assumed inputs are canonical representatives
// ModArithType ensures add/sub result can not overflow
struct ConvertAdd : public OpConversionPattern<AddOp> {
  ConvertAdd(mlir::MLIRContext *context)
      : OpConversionPattern<AddOp>(context) {}

  using OpConversionPattern::OpConversionPattern;

  LogicalResult matchAndRewrite(
      AddOp op, OpAdaptor adaptor,
      ConversionPatternRewriter &rewriter) const override {
    ImplicitLocOpBuilder b(op.getLoc(), rewriter);

    auto cmod = b.create<arith::ConstantOp>(modulusAttr(op));
    auto add = b.create<arith::AddIOp>(adaptor.getLhs(), adaptor.getRhs());
    auto remu = b.create<arith::RemUIOp>(add, cmod);

    rewriter.replaceOp(op, remu);
    return success();
  }
};

struct ConvertSub : public OpConversionPattern<SubOp> {
  ConvertSub(mlir::MLIRContext *context)
      : OpConversionPattern<SubOp>(context) {}

  using OpConversionPattern::OpConversionPattern;

  LogicalResult matchAndRewrite(
      SubOp op, OpAdaptor adaptor,
      ConversionPatternRewriter &rewriter) const override {
    ImplicitLocOpBuilder b(op.getLoc(), rewriter);

    auto cmod = b.create<arith::ConstantOp>(modulusAttr(op));
    auto sub = b.create<arith::SubIOp>(adaptor.getLhs(), adaptor.getRhs());
    auto add = b.create<arith::AddIOp>(sub, cmod);
    auto remu = b.create<arith::RemUIOp>(add, cmod);

    rewriter.replaceOp(op, remu);
    return success();
  }
};

struct ConvertMul : public OpConversionPattern<MulOp> {
  ConvertMul(mlir::MLIRContext *context)
      : OpConversionPattern<MulOp>(context) {}

  using OpConversionPattern::OpConversionPattern;

  LogicalResult matchAndRewrite(
      MulOp op, OpAdaptor adaptor,
      ConversionPatternRewriter &rewriter) const override {
    ImplicitLocOpBuilder b(op.getLoc(), rewriter);

    auto lhs =
        b.create<arith::ExtUIOp>(modulusType(op, true), adaptor.getLhs());
    auto rhs =
        b.create<arith::ExtUIOp>(modulusType(op, true), adaptor.getRhs());
    auto mul = b.create<arith::MulIOp>(lhs, rhs);

    auto modArithType = getResultModArithType(op);
    // here mul in [0, q^2], internal constraint
    auto bar = b.create<mod_arith::BarrettReduceOp>(modArithType, mul);
    auto subifge = b.create<mod_arith::SubIfGEOp>(modArithType, bar);

    rewriter.replaceOp(op, subifge);
    return success();
  }
};

struct ConvertMac : public OpConversionPattern<MacOp> {
  ConvertMac(mlir::MLIRContext *context)
      : OpConversionPattern<MacOp>(context) {}

  using OpConversionPattern::OpConversionPattern;

  LogicalResult matchAndRewrite(
      MacOp op, OpAdaptor adaptor,
      ConversionPatternRewriter &rewriter) const override {
    ImplicitLocOpBuilder b(op.getLoc(), rewriter);

    auto cmod = b.create<arith::ConstantOp>(modulusAttr(op, true));
    auto x = b.create<arith::ExtUIOp>(modulusType(op, true),
                                      adaptor.getOperands()[0]);
    auto y = b.create<arith::ExtUIOp>(modulusType(op, true),
                                      adaptor.getOperands()[1]);
    auto acc = b.create<arith::ExtUIOp>(modulusType(op, true),
                                        adaptor.getOperands()[2]);
    auto mul = b.create<arith::MulIOp>(x, y);
    auto add = b.create<arith::AddIOp>(mul, acc);
    auto remu = b.create<arith::RemUIOp>(add, cmod);
    auto trunc = b.create<arith::TruncIOp>(modulusType(op), remu);

    rewriter.replaceOp(op, trunc);
    return success();
  }
};

struct ConvertBarrettReduce : public OpConversionPattern<BarrettReduceOp> {
  ConvertBarrettReduce(mlir::MLIRContext *context)
      : OpConversionPattern<BarrettReduceOp>(context) {}

  using OpConversionPattern::OpConversionPattern;

  LogicalResult matchAndRewrite(
      BarrettReduceOp op, OpAdaptor adaptor,
      ConversionPatternRewriter &rewriter) const override {
    ImplicitLocOpBuilder b(op.getLoc(), rewriter);

    // Compute B = 2^{width} and ratio = floordiv(B / modulus)
    auto resultModArithType = getResultModArithType(op);
    APInt modulus = resultModArithType.getModulus().getValue();
    auto mulWidth = op.getInput().getType().getIntOrFloatBitWidth();
    auto width = modulus.getBitWidth();
    assert(2 * width == mulWidth);
    auto B = APInt(mulWidth, 1).shl(width);
    auto barrettRatio = B.udiv(modulus.zextOrTrunc(mulWidth));

    // Create our pre-computed constants
    // mul = true as we are operating on mulWidth
    TypedAttr ratioAttr = modulusAttr(op, true, barrettRatio);
    TypedAttr shiftAttr = modulusAttr(op, true, APInt(mulWidth, width));
    TypedAttr modAttr = modulusAttr(op, true);

    auto mulType = modulusType(op, true);

    auto ratioValue = b.create<arith::ConstantOp>(mulType, ratioAttr);
    auto shiftValue = b.create<arith::ConstantOp>(mulType, shiftAttr);
    auto modValue = b.create<arith::ConstantOp>(mulType, modAttr);

    // Compute x - floordiv(x * ratio, B) * mod
    auto mulRatioOp = b.create<arith::MulIOp>(adaptor.getInput(), ratioValue);
    auto shrOp = b.create<arith::ShRUIOp>(mulRatioOp, shiftValue);
    auto mulModOp = b.create<arith::MulIOp>(shrOp, modValue);
    auto subOp = b.create<arith::SubIOp>(adaptor.getInput(), mulModOp);

    auto truncOp = b.create<arith::TruncIOp>(modulusType(op, false), subOp);

    rewriter.replaceOp(op, truncOp);

    return success();
  }
};

struct ConvertSubIfGE : public OpConversionPattern<SubIfGEOp> {
  ConvertSubIfGE(mlir::MLIRContext *context)
      : OpConversionPattern<SubIfGEOp>(context) {}

  using OpConversionPattern::OpConversionPattern;

  LogicalResult matchAndRewrite(
      SubIfGEOp op, OpAdaptor adaptor,
      ConversionPatternRewriter &rewriter) const override {
    ImplicitLocOpBuilder b(op.getLoc(), rewriter);

    auto input = adaptor.getInput();
    auto cmod = b.create<arith::ConstantOp>(modulusAttr(op));
    auto sub = b.create<arith::SubIOp>(input, cmod);
    auto cmp = b.create<arith::CmpIOp>(arith::CmpIPredicate::uge, input, cmod);
    auto select = b.create<arith::SelectOp>(cmp, sub, input);

    rewriter.replaceOp(op, select);
    return success();
  }
};

struct ModArithToArith : impl::ModArithToArithBase<ModArithToArith> {
  using ModArithToArithBase::ModArithToArithBase;

  void runOnOperation() override;
};

void ModArithToArith::runOnOperation() {
  MLIRContext *context = &getContext();
  ModuleOp module = getOperation();
  ModArithToArithTypeConverter typeConverter(context);

  ConversionTarget target(*context);
  target.addIllegalDialect<ModArithDialect>();
  target.addLegalDialect<arith::ArithDialect>();

  RewritePatternSet patterns(context);
  patterns.add<ConvertEncapsulate, ConvertExtract, ConvertReduce, ConvertAdd,
               ConvertSub, ConvertMul, ConvertMac, ConvertBarrettReduce,
               ConvertSubIfGE>(typeConverter, context);

  addStructuralConversionPatterns(typeConverter, patterns, target);

  if (failed(applyPartialConversion(module, target, std::move(patterns)))) {
    signalPassFailure();
  }
}

}  // namespace mod_arith
}  // namespace heir
}  // namespace mlir
