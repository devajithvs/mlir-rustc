#include "AST/Expression.h"
#include "AST/LiteralExpression.h"
#include "ModuleBuilder/ModuleBuilder.h"

using namespace rust_compiler::ast;

namespace rust_compiler {

mlir::Value ModuleBuilder::emitExpression(std::shared_ptr<Expression> expr) {
  ExpressionKind kind = expr->getExpressionKind();

  llvm::outs() << "emitExpression"
               << "\n";

  switch (kind) {
  case ExpressionKind::ExpressionWithBlock: {
    return emitExpressionWithBlock(
        static_pointer_cast<rust_compiler::ast::ExpressionWithBlock>(expr));
  }
  case ExpressionKind::ExpressionWithoutBlock: {
    return emitExpressionWithoutBlock(
        static_pointer_cast<rust_compiler::ast::ExpressionWithoutBlock>(expr));
  }
  }
}

mlir::Value ModuleBuilder::emitLiteralExpression(
    std::shared_ptr<ast::LiteralExpression> lit) {
  assert(false);

  return nullptr;
}

void ModuleBuilder::emitReturnExpression(
    std::shared_ptr<ast::ReturnExpression> ret) {
  // mlir::Value reti = emitExpression(ret->getExpression());

  llvm::outs() << "emitReturnExpression"
               << "\n";

  if (ret->getExpression()) {
    mlir::Value expr = emitExpression(ret->getExpression());
    builder.create<mlir::func::ReturnOp>(getLocation(ret->getLocation()),
                                                expr);
  } else {
    builder.create<mlir::func::ReturnOp>(
        getLocation(ret->getLocation()));
  }
}

mlir::Value ModuleBuilder::emitOperatorExpression(
    std::shared_ptr<ast::OperatorExpression> opr) {
  assert(false);

  return nullptr;
}

} // namespace rust_compiler
