#pragma once

#include "AST/ArithmeticOrLogicalExpression.h"
#include "AST/ClosureExpression.h"
#include "AST/Crate.h"
#include "AST/Expression.h"
#include "AST/GenericParams.h"
#include "AST/Item.h"
#include "AST/LetStatement.h"
#include "AST/LiteralExpression.h"
#include "AST/MacroItem.h"
#include "AST/OperatorExpression.h"
#include "AST/PathExpression.h"
#include "AST/PathInExpression.h"
#include "AST/Patterns/PathPattern.h"
#include "AST/Patterns/PatternWithoutRange.h"
#include "AST/Patterns/RangePattern.h"
#include "AST/ReturnExpression.h"
#include "AST/Types/TypeExpression.h"
#include "AST/Types/TypePath.h"
#include "AST/WhereClause.h"
#include "Basic/Ids.h"
#include "Substitutions.h"
#include "TyCtx/TyCtx.h"
#include "TyTy.h"

// #include "../Resolver/Resolver.h"

#include <map>
#include <memory>
#include <vector>

namespace rust_compiler::sema::resolver {
class Resolver;
}

namespace rust_compiler::sema::type_checking {

// using namespace rust_compiler::sema::resolver;

/// https://doc.rust-lang.org/nightly/nightly-rustc/rustc_hir_analysis/index.html
class TypeResolver {
public:
  TypeResolver(resolver::Resolver *);

  void checkCrate(std::shared_ptr<ast::Crate> crate);

private:
  void checkVisItem(std::shared_ptr<ast::VisItem> v);
  void checkMacroItem(std::shared_ptr<ast::MacroItem> v);
  void checkFunction(std::shared_ptr<ast::Function> f);
  TyTy::BaseType *checkType(std::shared_ptr<ast::types::TypeExpression>);
  void checkWhereClause(const ast::WhereClause &);
  TyTy::BaseType *checkExpression(std::shared_ptr<ast::Expression>);
  TyTy::BaseType *
      checkExpressionWithBlock(std::shared_ptr<ast::ExpressionWithBlock>);
  TyTy::BaseType *
      checkExpressionWithoutBlock(std::shared_ptr<ast::ExpressionWithoutBlock>);
  TyTy::BaseType *checkBlockExpression(std::shared_ptr<ast::BlockExpression>);
  TyTy::BaseType *checkLiteral(std::shared_ptr<ast::LiteralExpression>);
  TyTy::BaseType *
      checkOperatorExpression(std::shared_ptr<ast::OperatorExpression>);
  TyTy::BaseType *checkArithmeticOrLogicalExpression(
      std::shared_ptr<ast::ArithmeticOrLogicalExpression>);
  TyTy::BaseType *checkReturnExpression(std::shared_ptr<ast::ReturnExpression>);
  void checkGenericParams(const ast::GenericParams &,
                          std::vector<TyTy::SubstitutionParamMapping> &);
  TyTy::BaseType *
      checkClosureExpression(std::shared_ptr<ast::ClosureExpression>);
  TyTy::BaseType *checkStatement(std::shared_ptr<ast::Statement>);
  TyTy::BaseType *checkPathExpression(std::shared_ptr<ast::PathExpression>);
  TyTy::BaseType *checkPathInExpression(std::shared_ptr<ast::PathInExpression>);
  TyTy::BaseType *checkLetStatement(std::shared_ptr<ast::LetStatement>);

  bool validateArithmeticType(ast::ArithmeticOrLogicalExpressionKind,
                              TyTy::BaseType *t);

  TyTy::BaseType *checkPattern(std::shared_ptr<ast::patterns::PatternNoTopAlt>,
                               TyTy::BaseType *);
  TyTy::BaseType *
  checkPatternWithoutRange(std::shared_ptr<ast::patterns::PatternWithoutRange>,
                           TyTy::BaseType *);
  TyTy::BaseType *
  checkRangePattern(std::shared_ptr<ast::patterns::RangePattern>,
                    TyTy::BaseType *);
  TyTy::BaseType *
  checkPathPattern(std::shared_ptr<ast::patterns::PathPattern>,
                    TyTy::BaseType *);

  TyTy::BaseType *checkTypeNoBounds(std::shared_ptr<ast::types::TypeNoBounds>);
  TyTy::BaseType *checkTypePath(std::shared_ptr<ast::types::TypePath>);

  TyTy::BaseType *resolveRootPath(std::shared_ptr<ast::types::TypePath> path,
                                  size_t *offset,
                                  basic::NodeId *resolvedNodeId);
  TyTy::BaseType *resolveSegments(basic::NodeId resolvedNodeId,
                                  basic::NodeId pathNodeId,
                                  std::shared_ptr<ast::types::TypePath> tp,
                                  size_t offset, TyTy::BaseType *pathType);

  std::optional<TyTy::BaseType *> queryType(basic::NodeId);

  // data
  tyctx::TyCtx *tcx;
  resolver::Resolver *resolver;
};

} // namespace rust_compiler::sema::type_checking
