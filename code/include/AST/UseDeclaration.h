#pragma once

#include "AST/VisItem.h"

#include <memory>

namespace rust_compiler::ast::use_tree {
class UseTree;
}

namespace rust_compiler::ast {

class UseDeclaration : public VisItem {
  std::shared_ptr<use_tree::UseTree> tree;

public:
  UseDeclaration(const adt::CanonicalPath &path,
                 rust_compiler::Location location)
      : VisItem(path, location, VisItemKind::UseDeclaration){};

  void setComponent(std::shared_ptr<use_tree::UseTree> tree);

  size_t getTokens() override;
};

} // namespace rust_compiler::ast
