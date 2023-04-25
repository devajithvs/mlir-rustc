#pragma once

#include "AST/AST.h"
#include "AST/GenericArg.h"

#include <vector>

namespace rust_compiler::ast {

class GenericArgs : public Node {
  std::vector<GenericArg> args;
  bool trailingComma = false;

public:
  GenericArgs(Location loc) : Node(loc) {}

  void addArg(const GenericArg &arg) { args.push_back(arg); }
  void setTrailingSemi() { trailingComma = true; }

  std::vector<GenericArg> getArgs() const { return args; }
};

} // namespace rust_compiler::ast
