#pragma once

#include <mlir/IR/BuiltinOps.h>
#include <mlir/IR/BuiltinTypes.h>
#include <mlir/IR/Dialect.h>
#include <mlir/IR/ExtensibleDialect.h>
#include <mlir/Interfaces/FunctionInterfaces.h> 
#include <mlir/IR/SymbolTable.h>
#include <mlir/Interfaces/CallInterfaces.h>
#include <mlir/Interfaces/CastInterfaces.h>
#include <mlir/Interfaces/SideEffectInterfaces.h>
#include <optional>

namespace rust_compiler::hir {

bool isScalarObject(mlir::Type);

bool isPattern(mlir::Type);
bool isPatternNoTopAlt(mlir::Type);

} // namespace rust_compiler::hir

#include "HirDialect.h.inc"
