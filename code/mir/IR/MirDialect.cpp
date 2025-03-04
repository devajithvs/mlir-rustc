#include "Mir/MirDialect.h"

#include "Mir/MirAttr.h"
#include "Mir/MirDialect.h"
#include "Mir/MirInterfaces.h"
#include "Mir/MirOps.h"
#include "Mir/MirTypes.h"

#include <llvm/Support/Debug.h>
#include <llvm/Support/WithColor.h>
#include <mlir/Dialect/Func/IR/FuncOps.h>
#include <mlir/IR/IRMapping.h>
#include <mlir/IR/Builders.h>
#include <mlir/IR/BuiltinTypes.h>
#include <mlir/IR/Dialect.h>
#include <mlir/IR/DialectImplementation.h>
#include <mlir/IR/OpImplementation.h>
#include <mlir/IR/Region.h>
#include <mlir/IR/Types.h>
#include <mlir/Transforms/InliningUtils.h>
#include <optional>

#define DEBUG_TYPE "MirDialect"

using namespace mlir;
using namespace llvm;
using namespace rust_compiler::Mir;

#include "Mir/MirDialect.cpp.inc"

// #define GET_OP_CLASSES
// #include "Mir/MirOps.cpp.inc"

struct MirInlinerInterface : public DialectInlinerInterface {
  using DialectInlinerInterface::DialectInlinerInterface;

  virtual ~MirInlinerInterface() = default;

  /// Returns true if the given operation 'callable', that implements the
  /// 'CallableOpInterface', can be inlined into the position given call
  /// operation 'call', that is registered to the current dialect and implements
  /// the `CallOpInterface`. 'wouldBeCloned' is set to true if the region of the
  /// given 'callable' is set to be cloned during the inlining process, or false
  /// if the region is set to be moved in-place(i.e. no duplicates would be
  /// created).
  bool isLegalToInline(Operation *call, Operation *callable,
                       bool wouldBeCloned) const override {
    CallableOpInterface callableI =
        llvm::dyn_cast<mlir::CallableOpInterface>(callable);
    if (!callableI)
      return false;

    CallOpInterface callI = llvm::dyn_cast<mlir::CallOpInterface>(call);
    if (!callI)
      return false;

    return false;
  }

  /// Returns true if the given region 'src' can be inlined into the region
  /// 'dest' that is attached to an operation registered to the current dialect.
  /// 'wouldBeCloned' is set to true if the given 'src' region is set to be
  /// cloned during the inlining process, or false if the region is set to be
  /// moved in-place(i.e. no duplicates would be created). 'valueMapping'
  /// contains any remapped values from within the 'src' region. This can be
  /// used to examine what values will replace entry arguments into the 'src'
  /// region for example.
  bool isLegalToInline(mlir::Region *reg, Region *dest, bool wouldBeCloned,
                       mlir::IRMapping &irMapping) const override {
    return false;
  }

  void handleTerminator(Operation *op,
                        ArrayRef<Value> valuesToReplace) const override {
    // Only "Mir.return" needs to be handled here.
    auto returnOp = llvm::cast<mlir::func::ReturnOp>(op);

    // Replace the values directly with the return operands.
    assert(returnOp.getNumOperands() == valuesToReplace.size());
    for (const auto &it : llvm::enumerate(returnOp.getOperands()))
      valuesToReplace[it.index()].replaceAllUsesWith(it.value());
  }

  //  Operation *materializeCallConversion(OpBuilder &builder, Value input,
  //                                       Type resultType,
  //                                       Location conversionLoc) const
  //                                       override {
  //    return builder.create<CastOp>(conversionLoc, resultType, input);
  //  }
};

void MirDialect::initialize() {
  addOperations<
#define GET_OP_LIST
#include "Mir/MirOps.cpp.inc"
      >();
  addInterfaces<MirInlinerInterface>();
  addTypes<StructType>();
}

namespace rust_compiler::Mir {

/// Create an instance of a `StructType` with the given element types. There
/// *must* be at least one element type.
StructType StructType::get(llvm::ArrayRef<mlir::Type> elementTypes) {
  assert(!elementTypes.empty() && "expected at least 1 element type");

  // Call into a helper 'get' method in 'TypeBase' to get a uniqued instance
  // of this type. The first parameter is the context to unique in. The
  // parameters after the context are forwarded to the storage instance.
  mlir::MLIRContext *ctx = elementTypes.front().getContext();
  return Base::get(ctx, elementTypes);
}

/// Returns the element types of this struct type.
llvm::ArrayRef<mlir::Type> StructType::getElementTypes() {
  // 'getImpl' returns a pointer to the internal storage instance.
  return getImpl()->elementTypes;
}

} // namespace rust_compiler::Mir
