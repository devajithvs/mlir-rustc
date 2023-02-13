#pragma once

#include "AST/GenericParams.h"
#include "AST/Struct.h"
#include "AST/TupleFields.h"
#include "AST/WhereClause.h"

#include <optional>
#include <string>

namespace rust_compiler::ast {

class TupleStruct : public Struct {
  std::string identifier;
  std::optional<GenericParams> genericParms;
  std::optional<TupleFields> tupleFields;
  std::optional<WhereClause> whereClause;

public:
  TupleStruct(Location loc, std::optional<Visibility> vis)
    : Struct(loc, StructKind::TupleStruct, vis) {}
};

} // namespace rust_compiler::ast
