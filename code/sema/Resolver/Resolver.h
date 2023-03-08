#pragma once

#include "ADT/CanonicalPath.h"
#include "ADT/ScopedCanonicalPath.h"
#include "AST/Crate.h"
#include "AST/Expression.h"
#include "AST/Implementation.h"
#include "AST/InherentImpl.h"
#include "AST/MacroItem.h"
#include "AST/Patterns/PatternNoTopAlt.h"
#include "AST/StaticItem.h"
#include "AST/TraitImpl.h"
#include "AST/Types/TypeExpression.h"
#include "AST/UseDeclaration.h"
#include "AST/VisItem.h"
#include "AST/Visiblity.h"
#include "Basic/Ids.h"

#include "../TypeChecking/TypeChecking.h"

#include <map>
#include <optional>
#include <stack>
#include <string_view>
#include <vector>

namespace rust_compiler::sema::resolver {

/// https://doc.rust-lang.org/nightly/nightly-rustc/rustc_resolve/late/struct.Rib.html
class Rib {
public:
  // https://doc.rust-lang.org/nightly/nightly-rustc/rustc_resolve/late/enum.RibKind.html
  enum class RibKind { Param, Type };

  Rib(RibKind kind) : kind(kind) {}

  RibKind getKind() const { return kind; }

private:
  std::map<std::string, basic::NodeId> bindings;
  RibKind kind;
};

class Scope {
public:
  Scope(basic::CrateNum crateNum);

  Rib *peek();
  void push(basic::NodeId id);
  Rib *pop();

  basic::CrateNum getCrateNum() const { return crateNum; }

private:
  basic::CrateNum crateNum;
  basic::NodeId node_id;
  std::vector<Rib *> stack;
};

class Segment {
  std::string name;
};

class Import {
public:
  enum class ImportKind { Single, Glob, ExternCrate, MacroUse, MacroExport };

  ImportKind getKind() const { return kind; }

  basic::NodeId getNodeId() const { return nodeId; }

private:
  ImportKind kind;
  basic::NodeId nodeId;
  llvm::SmallVector<Segment> modulePath;
};

class Resolver {
public:
  Resolver() noexcept;

  ~Resolver() = default;

  void resolveCrate(std::shared_ptr<ast::Crate>);

private:
  // adt::ScopedCanonicalPath scopedPath;

  // items no recurse
  void resolveVisItemNoRecurse(std::shared_ptr<ast::VisItem>,
                               const adt::CanonicalPath &prefix,
                               const adt::CanonicalPath &canonicalPrefix);
  void resolveMacroItemNoRecurse(std::shared_ptr<ast::MacroItem>,
                                 const adt::CanonicalPath &prefix,
                                 const adt::CanonicalPath &canonicalPrefix);
  void resolveFunctionNoRecurse(std::shared_ptr<ast::Function>,
                                const adt::CanonicalPath &prefix,
                                const adt::CanonicalPath &canonicalPrefix);

  // items
  void resolveVisItem(std::shared_ptr<ast::VisItem>,
                      const adt::CanonicalPath &prefix,
                      const adt::CanonicalPath &canonicalPrefix);
  void resolveMacroItem(std::shared_ptr<ast::MacroItem>,
                        const adt::CanonicalPath &prefix,
                        const adt::CanonicalPath &canonicalPrefix);
  void resolveStaticItem(std::shared_ptr<ast::StaticItem>,
                         const adt::CanonicalPath &prefix,
                         const adt::CanonicalPath &canonicalPrefix);
  void resolveConstantItem(std::shared_ptr<ast::ConstantItem>,
                           const adt::CanonicalPath &prefix,
                           const adt::CanonicalPath &canonicalPrefix);
  void resolveImplementation(std::shared_ptr<ast::Implementation>,
                             const adt::CanonicalPath &prefix,
                             const adt::CanonicalPath &canonicalPrefix);
  void resolveUseDeclaration(std::shared_ptr<ast::UseDeclaration>,
                             const adt::CanonicalPath &prefix,
                             const adt::CanonicalPath &canonicalPrefix);
  void resolveInherentImpl(std::shared_ptr<ast::InherentImpl>,
                           const adt::CanonicalPath &prefix,
                           const adt::CanonicalPath &canonicalPrefix);
  void resolveTraitImpl(std::shared_ptr<ast::TraitImpl>,
                        const adt::CanonicalPath &prefix,
                        const adt::CanonicalPath &canonicalPrefix);
  void resolveFunction(std::shared_ptr<ast::Function>,
                       const adt::CanonicalPath &prefix,
                       const adt::CanonicalPath &canonicalPrefix);
  void resolveModule(std::shared_ptr<ast::Module>,
                     const adt::CanonicalPath &prefix,
                     const adt::CanonicalPath &canonicalPrefix);

  // expressions
  void resolveExpression(std::shared_ptr<ast::Expression>,
                         const adt::CanonicalPath &prefix,
                         const adt::CanonicalPath &canonicalPrefix);
  void resolveExpressionWithBlock(std::shared_ptr<ast::ExpressionWithBlock>,
                                  const adt::CanonicalPath &prefix,
                                  const adt::CanonicalPath &canonicalPrefix);
  void
  resolveExpressionWithoutBlock(std::shared_ptr<ast::ExpressionWithoutBlock>,
                                const adt::CanonicalPath &prefix,
                                const adt::CanonicalPath &canonicalPrefix);

  // types
  void resolveType(std::shared_ptr<ast::types::TypeExpression>);

  // checks
  void resolveVisibility(std::optional<ast::Visibility>);

  // generics
  void resolveWhereClause(const ast::WhereClause &);
  void resolveGenericParams(const ast::GenericParams &,
                            const adt::CanonicalPath &prefix,
                            const adt::CanonicalPath &canonicalPrefix);

  // patterns
  void
      resolvePatternDeclaration(std::shared_ptr<ast::patterns::PatternNoTopAlt>,
                                Rib::RibKind);

  std::map<basic::NodeId, std::shared_ptr<ast::UseDeclaration>> useDeclarations;
  std::map<basic::NodeId, std::shared_ptr<ast::Module>> modules;

  std::vector<Import> determinedImports;

  //  std::map<adt::CanonicalPath,
  //           std::pair<std::shared_ptr<ast::Module>, basic::NodeId>>
  //      modules;

  void pushNewModuleScope(basic::NodeId moduleId) {
    currentModuleStack.push_back(moduleId);
  }

  void popModuleScope() { currentModuleStack.pop_back(); }

  basic::NodeId peekCurrentModuleScope() const {
    return currentModuleStack.back();
  }

  Mappings *mappings;

  // types
  type_checking::TypeCheckContext *tyctx;
  void generateBuiltins();

  // Scopes
  Scope &getNameScope() { return nameScope; }
  Scope &getTypeScope() { return typeScope; }
  Scope &getLabelScope() { return labelScope; }
  Scope &getMacroScope() { return macroScope; }

  Scope nameScope;
  Scope typeScope;
  Scope labelScope;
  Scope macroScope;

  // Ribs
  void pushNewNameRib(Rib *);
  void pushNewTypeRib(Rib *);
  void pushNewLabelRib(Rib *);
  void pushNewMaroRib(Rib *);

  // keep track of the current module scope ids
  std::vector<basic::NodeId> currentModuleStack;

  // Rest
  basic::NodeId globalTypeNodeId;
  basic::NodeId unitTyNodeId;
};

} // namespace rust_compiler::sema::resolver

// FIXME: Scoped
// FIXME: store canonical paths
