#include "AST/LiteralExpression.h"

namespace rust_compiler::ast {

size_t LiteralExpression::getTokens() {

  if (kind == LiteralExpressionKind::IntegerLiteral)
    return 1;

  if (kind == LiteralExpressionKind::True)
    return 1;

  if (kind == LiteralExpressionKind::False)
    return 1;

  assert(false);

  return 1;
}

} // namespace rust_compiler::ast
