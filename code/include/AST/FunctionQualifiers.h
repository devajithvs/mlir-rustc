#pragma once

#include "AST/AST.h"

#include <string>
#include <string_view>

namespace rust_compiler::ast {

enum class FunctionQualifierKind { Const, Async, Unsafe, Extern };

class FunctionQualifiers : public Node {
  FunctionQualifierKind kind;
  std::string abi;

public:
  FunctionQualifierKind getKind() const { return kind; }

  size_t getTokens() override;

  void setAsync();
  void setConst();
  void setUnsafe();
  void setExtern(std::string_view abi);

};

} // namespace rust_compiler::ast
