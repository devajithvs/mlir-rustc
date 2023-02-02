#pragma once

#include "ADT/CanonicalPath.h"

#include <stack>
#include <string>
#include <string_view>

namespace rust_compiler::adt {

class ScopedCanonicalPath;

class ScopedCanonicalPathScope {
  ScopedCanonicalPath *parent;

public:
  ScopedCanonicalPathScope(ScopedCanonicalPath *storage,
                           std::string_view segment);

  ~ScopedCanonicalPathScope();
};

class ScopedCanonicalPath {
  using ScopeTy = ScopedCanonicalPathScope;

  std::stack<ScopeTy *> scopes;

  std::stack<std::string> segments;
  std::string crateName;

public:
  ScopedCanonicalPath(const CanonicalPath &path){};

  CanonicalPath getCurrentPath() const;

private:
  friend ScopedCanonicalPathScope;

  void registerScope(ScopedCanonicalPathScope *, std::string_view segment);
  void deregisterScope(ScopedCanonicalPathScope *);
};

} // namespace rust_compiler::adt
