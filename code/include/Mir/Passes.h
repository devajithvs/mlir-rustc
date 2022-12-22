#pragma once

#include <memory>

namespace mlir {
class Pass;
}

namespace rust_compiler {

std::unique_ptr<mlir::Pass> createLowerToLLVMPass();

}
