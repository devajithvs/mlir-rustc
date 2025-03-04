#ifndef HirOpsBase
#define HirOpsBase

include "mlir/IR/AttrTypeBase.td"

def IsScalarPred
      : CPred<"rust_compiler::hir::isScalarObject($_self)">;
def AnyScalarObject: Type<IsScalarPred, "any scalar-like object">;

#endif
