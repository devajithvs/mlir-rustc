#include "ADT/CanonicalPath.h"
#include "AST/Function.h"
#include "AST/GenericParams.h"
#include "AST/Struct.h"
#include "AST/StructField.h"
#include "AST/VisItem.h"
#include "TyCtx/TyTy.h"
#include "TyCtx/TypeIdentity.h"
#include "TypeChecking.h"

#include <memory>
#include <optional>
#include <vector>

using namespace rust_compiler::ast;
using namespace rust_compiler::tyctx;
using namespace rust_compiler::adt;

namespace rust_compiler::sema::type_checking {

void TypeResolver::checkVisItem(std::shared_ptr<ast::VisItem> v) {
  switch (v->getKind()) {
  case VisItemKind::Module: {
    assert(false && "to be implemented");
  }
  case VisItemKind::ExternCrate: {
    assert(false && "to be implemented");
  }
  case VisItemKind::UseDeclaration: {
    assert(false && "to be implemented");
  }
  case VisItemKind::Function: {
    checkFunction(std::static_pointer_cast<Function>(v));
    break;
  }
  case VisItemKind::TypeAlias: {
    assert(false && "to be implemented");
  }
  case VisItemKind::Struct: {
    checkStruct(static_cast<ast::Struct *>(v.get()));
    break;
  }
  case VisItemKind::Enumeration: {
    assert(false && "to be implemented");
  }
  case VisItemKind::Union: {
    assert(false && "to be implemented");
  }
  case VisItemKind::ConstantItem: {
    assert(false && "to be implemented");
  }
  case VisItemKind::StaticItem: {
    assert(false && "to be implemented");
  }
  case VisItemKind::Trait: {
    assert(false && "to be implemented");
  }
  case VisItemKind::Implementation: {
    assert(false && "to be implemented");
  }
  case VisItemKind::ExternBlock: {
    assert(false && "to be implemented");
  }
  }
}

void TypeResolver::checkMacroItem(std::shared_ptr<ast::MacroItem> v) {
  assert(false && "to be implemented");
}

void TypeResolver::checkStruct(ast::Struct *s) {
  switch (s->getKind()) {
  case StructKind::StructStruct2: {
    checkStructStruct(static_cast<StructStruct *>(s));
    break;
  }
  case StructKind::TupleStruct2: {
    checkTupleStruct(static_cast<TupleStruct *>(s));
    break;
  }
  }
}

void TypeResolver::checkStructStruct(ast::StructStruct *s) {

  std::optional<GenericParams> genericParams;
  if (s->hasGenerics()) {
    genericParams = s->getGenericParams();
    checkGenericParams(s->getGenericParams());
  }

  if (s->hasWhereClause())
    checkWhereClause(s->getWhereClause());

  std::vector<TyTy::StructFieldType *> fields;

  if (s->hasStructFields()) {
    for (StructField &field : s->getFields().getFields()) {
      TyTy::BaseType *fieldType = checkType(field.getType());
      TyTy::StructFieldType *strField =
          new TyTy::StructFieldType(field.getNodeId(), field.getIdentifier(),
                                    fieldType, field.getLocation());
      fields.push_back(strField);
      tcx->insertType(field.getIdentity(), fieldType);
    }
  }

  std::optional<adt::CanonicalPath> path =
      tcx->lookupCanonicalPath(s->getNodeId());
  assert(path.has_value());
  tyctx::TypeIdentity ident = {*path, s->getLocation()};

  std::vector<TyTy::VariantDef *> variants;

  variants.push_back(new TyTy::VariantDef(s->getNodeId(), s->getIdentifier(),
                                          ident, TyTy::VariantKind::Struct,
                                          nullptr, fields));

  // parse #[repr(X)]
  TyTy::BaseType *type =
      new TyTy::ADTType(s->getNodeId(), s->getIdentifier(), ident,
                        TyTy::ADTKind::StructStruct, variants, genericParams);

  tcx->insertType(s->getIdentity(), type);
}

void TypeResolver::checkTupleStruct(ast::TupleStruct *s) {
  std::optional<GenericParams> genericParams;
  if (s->hasGenerics()) {
    genericParams = s->getGenericParams();
    checkGenericParams(s->getGenericParams());
  }

  if (s->hasWhereClause())
    checkWhereClause(s->getWhereClause());

  std::vector<TyTy::StructFieldType *> fields;

  size_t idx = 0;
  if (s->hasTupleFields()) {
    for (TupleField &field : s->getTupleFields().getFields()) {
      TyTy::BaseType *fieldType = checkType(field.getType());
      TyTy::StructFieldType *fiel = new TyTy::StructFieldType(
          field.getNodeId(), Identifier(std::to_string(idx)), fieldType,
          field.getLocation());
      fields.push_back(fiel);
      tcx->insertType(field.getIdentity(), fieldType);
      ++idx;
    }
  }

  std::optional<CanonicalPath> path = tcx->lookupCanonicalPath(s->getNodeId());
  assert(path.has_value());
  tyctx::TypeIdentity ident = {*path, s->getLocation()};

  std::vector<TyTy::VariantDef *> variants;
  variants.push_back(new TyTy::VariantDef(s->getNodeId(), s->getName(), ident,
                                          TyTy::VariantKind::Tuple, nullptr, fields));

  // parse #[rept(X)]

  TyTy::BaseType *type =
      new TyTy::ADTType(s->getNodeId(), s->getName(), ident,
                        TyTy::ADTKind::TupleStruct, variants, genericParams);

  tcx->insertType(s->getIdentity(), type);
}

} // namespace rust_compiler::sema::type_checking
