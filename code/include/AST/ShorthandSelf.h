#pragma once

#include "AST/AST.h"
#include "AST/SelfParam.h"

#include "Location.h"

namespace rust_compiler::ast {

class ShorthandSelf : public SelfParam {

public:
  ShorthandSelf(Location loc) : SelfParam(loc) {}

  void setMut();
  void setAnd();
};

} // namespace rust_compiler::ast
