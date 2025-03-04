
#include "Hir/HirDialect.h"

#include "Hir/HirEnum.h"
#include "Hir/HirInterfaces.h"
#include "Hir/HirOps.h"
#include "Hir/HirString.h"
#include "Hir/HirTypes.h"

#include <mlir/Dialect/Arith/IR/Arith.h>
#include <mlir/Dialect/ControlFlow/IR/ControlFlow.h>
#include <mlir/Dialect/MemRef/IR/MemRef.h>
#include <mlir/Dialect/Vector/IR/VectorOps.h>
#include <mlir/IR/IRMapping.h>
#include <mlir/IR/Builders.h>
#include <mlir/IR/BuiltinTypes.h>
#include <mlir/IR/Dialect.h>
#include <mlir/IR/DialectImplementation.h>
#include <mlir/IR/ExtensibleDialect.h>
#include <mlir/IR/OpImplementation.h>
#include <mlir/IR/Region.h>
#include <mlir/IR/Types.h>
#include <mlir/Transforms/InliningUtils.h>
#include <optional>

#include <llvm/ADT/ArrayRef.h>

using namespace mlir;

#include "Hir/HirDialect.cpp.inc"

namespace rust_compiler::hir {

class HirInlinerInterface
    : public DialectInterface::Base<DialectInlinerInterface> {
public:
};

bool isScalarObject(mlir::Type type) {
  if (mlir::IntegerType integer = mlir::dyn_cast<mlir::IntegerType>(type))
    return true;
  if (mlir::FloatType floatTy = mlir::dyn_cast<mlir::FloatType>(type))
    return true;
  return false;
}

void HirDialect::initialize() {
  registerTypes();
  addOperations<
#define GET_OP_LIST
#include "Hir/HirOps.cpp.inc"
      >();
}

} // namespace rust_compiler::hir
