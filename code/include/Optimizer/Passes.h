#pragma once

#include <memory>
#include <mlir/Pass/Pass.h>

namespace rust_compiler::optimizer {

#define GEN_PASS_DECL_TEST
#include "Optimizer/Passes.h.inc"

#define GEN_PASS_DECL_ATTRIBUTERLITE
#include "Optimizer/Passes.h.inc"

#define GEN_PASS_DECL_REWRITEPASS
#include "Optimizer/Passes.h.inc"

#define GEN_PASS_DECL_LOWERAWAITPASS
#include "Optimizer/Passes.h.inc"

#define GEN_PASS_DECL_LOWERERRORPROPAGATIONPASS
#include "Optimizer/Passes.h.inc"

std::unique_ptr<mlir::Pass> createTestPass();
std::unique_ptr<mlir::Pass> createAttributerPass();
std::unique_ptr<mlir::Pass> createRewriterPass();
std::unique_ptr<mlir::Pass> createLowerAwaitPass();
std::unique_ptr<mlir::Pass> createLowerToLLVMPass();
std::unique_ptr<mlir::Pass> createLowerErrorPropagationPass();

// declarative passes
#define GEN_PASS_REGISTRATION
#include "Optimizer/Passes.h.inc"

} // namespace rust_compiler::optimizer
