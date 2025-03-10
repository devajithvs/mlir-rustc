#include "Analysis/Attributer/Attributer.h"

#include "Optimizer/Passes.h"

#include <mlir/Dialect/Func/IR/FuncOps.h>
#include <mlir/Pass/Pass.h>
#include <mlir/IR/BuiltinOps.h>

using namespace mlir;

namespace rust_compiler::optimizer {
#define GEN_PASS_DEF_ATTRIBUTER
#include "Optimizer/Passes.h.inc"
} // namespace rust_compiler::optimizer

namespace {
class AttributerPass
    : public rust_compiler::optimizer::impl::AttributerBase<AttributerPass> {
public:
  void runOnOperation() override;
};

} // namespace

using namespace rust_compiler::analysis::attributor;

void AttributerPass::runOnOperation() {
  ModuleOp module = getOperation();

  Attributor attr = {module};

  attr.setup();

  //mlir::LogicalResult result = attr.run();

  // mlir::ModuleOp module = getOperation();
  //  module.walk([&](mlir::func::FuncOp *f) {
  //  });
}

// https://reviews.llvm.org/D140415
