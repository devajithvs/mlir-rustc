#pragma once

#include "AST/AST.h"

#include <memory>
#include <vector>

namespace rust_compiler::ast::patterns {

class PatternNoTopAlt;

class Pattern : public Node {
  std::vector<std::shared_ptr<ast::patterns::PatternNoTopAlt>> patterns;

public:
  Pattern(Location loc) : Node(loc) {}

  void addPattern(std::shared_ptr<ast::patterns::PatternNoTopAlt>);
};

} // namespace rust_compiler::ast::patterns

// https://doc.rust-lang.org/reference/patterns.html
