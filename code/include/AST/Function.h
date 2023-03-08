#pragma once

#include "AST/FunctionParameters.h"
#include "AST/FunctionQualifiers.h"
#include "AST/FunctionReturnType.h"
#include "AST/GenericParams.h"
#include "AST/VisItem.h"
#include "AST/WhereClause.h"

#include <memory>
#include <string>

namespace rust_compiler::ast {

class BlockExpression;

class Function : public VisItem {
  std::shared_ptr<Expression> body;
  FunctionQualifiers qualifiers;
  FunctionParameters functionParameters;
  GenericParams genericParams;
  std::optional<FunctionReturnType> returnType;
  WhereClause whereClause;

  std::string identifier;

public:
  Function(Location loc, std::optional<Visibility> vis)
      : VisItem(loc, VisItemKind::Function, vis), qualifiers(loc),
        functionParameters(loc), genericParams(loc), whereClause(loc) {}

  bool hasBody() const;

  std::shared_ptr<Expression> getBody();

  void setQualifiers(FunctionQualifiers qualifiers);

  void setGenericParams(GenericParams genericParams);

  void setWhereClasue(WhereClause whereClause);

  void setParameters(FunctionParameters functionParameters);

  void setBody(std::shared_ptr<Expression> block);

  void setReturnType(const FunctionReturnType &ret) { returnType = ret; }

  void setIdentifier(std::string_view id) { identifier = id; }

  std::string_view getName() const { return identifier; }
};

} // namespace rust_compiler::ast

// BlockExpression
