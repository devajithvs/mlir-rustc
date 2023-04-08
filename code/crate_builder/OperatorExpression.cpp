#include "AST/OperatorExpression.h"

#include "AST/ArithmeticOrLogicalExpression.h"
#include "CrateBuilder/CrateBuilder.h"

#include <memory>

using namespace rust_compiler::ast;

namespace rust_compiler::crate_builder {

mlir::Value
CrateBuilder::emitOperatorExpression(ast::OperatorExpression *expr) {

  switch (expr->getKind()) {
  case OperatorExpressionKind::BorrowExpression: {
    break;
  }
  case OperatorExpressionKind::DereferenceExpression: {
    break;
  }
  case OperatorExpressionKind::ErrorPropagationExpression: {
    break;
  }
  case OperatorExpressionKind::NegationExpression: {
    break;
  }
  case OperatorExpressionKind::ArithmeticOrLogicalExpression: {
    return emitArithmeticOrLogicalExpression(
        static_cast<ArithmeticOrLogicalExpression *>(expr));
    break;
  }
  case OperatorExpressionKind::ComparisonExpression: {
    break;
  }
  case OperatorExpressionKind::LazyBooleanExpression: {
    break;
  }
  case OperatorExpressionKind::TypeCastExpression: {
    break;
  }
  case OperatorExpressionKind::AssignmentExpression: {
    break;
  }
  case OperatorExpressionKind::CompoundAssignmentExpression: {
    break;
  }
  }
  assert(false);
}

mlir::Value CrateBuilder::emitArithmeticOrLogicalExpression(
    ArithmeticOrLogicalExpression *expr) {
  switch (expr->getKind()) {
  case ArithmeticOrLogicalExpressionKind::Addition: {
    break;
  }
  case ArithmeticOrLogicalExpressionKind::Subtraction: {
    break;
  }
  case ArithmeticOrLogicalExpressionKind::Multiplication: {
    break;
  }
  case ArithmeticOrLogicalExpressionKind::Division: {
    break;
  }
  case ArithmeticOrLogicalExpressionKind::Remainder: {
    break;
  }
  case ArithmeticOrLogicalExpressionKind::BitwiseAnd: {
    break;
  }
  case ArithmeticOrLogicalExpressionKind::BitwiseOr: {
    break;
  }
  case ArithmeticOrLogicalExpressionKind::BitwiseXor: {
    break;
  }
  case ArithmeticOrLogicalExpressionKind::LeftShift: {
    break;
  }
  case ArithmeticOrLogicalExpressionKind::RightShift: {
    break;
  }
  }
  assert(false);
}

} // namespace rust_compiler::crate_builder
