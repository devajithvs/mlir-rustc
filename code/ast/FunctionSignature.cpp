#include "AST/FunctionSignature.h"

#include "AST/GenericParams.h"
#include "AST/Types/TypeExpression.h"
#include "AST/WhereClause.h"

namespace rust_compiler::ast {

std::string FunctionSignature::getName() { return name; }

void FunctionSignature::setName(std::string_view _name) { name = _name; }

void FunctionSignature::setQualifiers(FunctionQualifiers _qual) {
  qual = _qual;
}

void FunctionSignature::setParameters(FunctionParameters _parm) {
  parm = _parm;
}

void FunctionSignature::setReturnType(
    std::shared_ptr<ast::types::TypeExpression> _returnType) {
  returnType = _returnType;
}

bool FunctionSignature::hasReturnType() const { return (bool)returnType; }

void FunctionSignature::setWhereClause(std::shared_ptr<WhereClause> _where) {
  where = _where;
}

void FunctionSignature::setGenericParams(std::shared_ptr<GenericParams> _gen) {
  gen = _gen;
}

} // namespace rust_compiler::ast
