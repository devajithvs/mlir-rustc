#include "Expression.h"

#include "Parser/Parser.h"

#include <llvm/Support/raw_ostream.h>

using namespace rust_compiler::ast;
using namespace rust_compiler::lexer;

namespace rust_compiler::parser {

std::optional<std::shared_ptr<ast::Expression>>
Parser::tryParseExpression(std::span<lexer::Token> tokens) {
  std::span<lexer::Token> view = tokens;

  llvm::errs() << "tryParseExpression: " << Token2String(view.front().getKind())
               << "\n";

  std::optional<std::shared_ptr<ast::Expression>> woBlock =
      tryParseExpressionWithoutBlock(view);
  if (woBlock) {
    llvm::errs() << "tryParseExpression: found woBlock"
                 << "\n";
    return woBlock;
  }

  std::optional<std::shared_ptr<ast::Expression>> withBlock =
      tryParseExpressionWithBlock(view);
  if (withBlock) {
    llvm::errs() << "tryParseExpression: found withBlock"
                 << "\n";
    return withBlock;
  }

  return std::nullopt;
}

} // namespace rust_compiler::parser
