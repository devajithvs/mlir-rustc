#include "TyCtx/TyCtx.h"

#include "ADT/CanonicalPath.h"
#include "AST/AssociatedItem.h"
#include "AST/Crate.h"
#include "AST/EnumItem.h"
#include "AST/ExternalItem.h"
#include "AST/Types/TypePath.h"
#include "Basic/Ids.h"

// #include "../sema/TypeChecking/TypeChecking.h"

#include <memory>
#include <optional>

using namespace rust_compiler::basic;
using namespace rust_compiler::ast;
using namespace rust_compiler::ast::types;
using namespace rust_compiler::adt;
// using namespace rust_compiler::sema::type_checking;

namespace rust_compiler::tyctx {

TyCtx::TyCtx() { generateBuiltins(); }

std::optional<std::string> TyCtx::getCrateName(CrateNum cnum) {
  auto it = astCrateMappings.find(cnum);
  if (it == astCrateMappings.end())
    return std::nullopt;

  return it->second->getCrateName();
}

// TyCtx *TyCtx::get() {
//   static std::unique_ptr<TyCtx> instance;
//   if (!instance)
//     instance = std::unique_ptr<TyCtx>(new TyCtx());
//
//   return instance.get();
// }

// NodeId TyCtx::getNextNodeId() {
//   auto it = nodeIdIter;
//   ++nodeIdIter;
//   return it;
// }

void TyCtx::insertResolvedName(NodeId ref, NodeId def) {
  resolvedNames[ref] = def;
}

std::optional<NodeId> TyCtx::lookupName(NodeId ref) {
  auto it = resolvedNames.find(ref);
  if (it == resolvedNames.end())
    return std::nullopt;
  return it->second;
}

std::optional<TyTy::TypeBoundPredicate> TyCtx::lookupPredicate(NodeId) {
  assert(false);
}

void TyCtx::insertModule(ast::Module *mod) { assert(false); }

ast::Module *TyCtx::lookupModule(basic::NodeId id) {
  auto it = modules.find(id);
  if (it == modules.end())
    return nullptr;
  return it->second;
}

basic::CrateNum TyCtx::getCurrentCrate() const { return currentCrateNum; }

void TyCtx::setCurrentCrate(basic::CrateNum crateNum) {
  currentCrateNum = crateNum;
}

bool TyCtx::isModule(NodeId id) {
  return moduleChildItems.find(id) != moduleChildItems.end();
}

std::optional<adt::CanonicalPath>
TyCtx::lookupModuleChild(NodeId module, const adt::CanonicalPath &item) {
  std::optional<std::vector<adt::CanonicalPath>> children =
      lookupModuleChildrenItems(module);
  if (!children)
    return std::nullopt;

  for (auto &child : *children) {
    if (child.isEqualByName(item))
      return child;
  }
  return std::nullopt;
}

std::optional<std::vector<adt::CanonicalPath>>
TyCtx::lookupModuleChildrenItems(basic::NodeId module) {
  auto it = moduleChildItems.find(module);
  if (it == moduleChildItems.end())
    return std::nullopt;

  return it->second;
}

bool TyCtx::isCrate(NodeId nod) const {
  for (const auto &it : astCrateMappings) {
    NodeId crateNodeId = it.second->getNodeId();
    if (crateNodeId == nod)
      return true;
  }
  return false;
}

void TyCtx::insertASTCrate(ast::Crate *crate, CrateNum crateNum) {
  astCrateMappings.insert({crateNum, crate});
}

void TyCtx::insertBuiltin(NodeId id, NodeId ref, TyTy::BaseType *type) {
  nodeIdRefs[ref] = id;
  resolved[id] = type;
  builtinsList.push_back(std::unique_ptr<TyTy::BaseType>(type));
}

void TyCtx::insertType(const NodeIdentity &id, TyTy::BaseType *type) {
  resolved[id.getNodeId()] = type;
}

TyTy::BaseType *TyCtx::lookupBuiltin(std::string_view name) {
  for (auto &built : builtinsList) {
    if (built->toString() == name) {
      return built.get();
    }
  }

  return nullptr;
}

std::optional<TyTy::BaseType *> TyCtx::lookupType(basic::NodeId id) {
  auto it = resolved.find(id);
  if (it != resolved.end())
    return it->second;
  return std::nullopt;
}

void TyCtx::insertItem(ast::Item *it) { itemMappings[it->getNodeId()] = it; }

std::optional<ast::Item *> TyCtx::lookupItem(basic::NodeId id) {
  auto it = itemMappings.find(id);
  if (it == itemMappings.end())
    return std::nullopt;

  return it->second;
}

std::optional<ast::ExternalItem *> TyCtx::lookupExternalItem(basic::NodeId id) {
  auto it = externItemMappings.find(id);
  if (it == externItemMappings.end())
    return std::nullopt;

  return it->second.first;
}

// void Resolver::insertImplementation(NodeId id, ast::Implementation *impl) {
//
// }
//
// std::optional<ast::Implementation *>
// TyCtx::lookupImplementation(basic::NodeId id) {
//   assert(false);
// }

std::optional<std::pair<ast::Enumeration *, ast::EnumItem *>>
TyCtx::lookupEnumItem(NodeId id) {
  auto it = enumItemsMappings.find(id);
  if (it == enumItemsMappings.end())
    return std::nullopt;

  return it->second;
}

void TyCtx::insertEnumItem(ast::Enumeration *parent, ast::EnumItem *item) {
  item->getNodeId();
  auto enumItem = lookupEnumItem(item->getNodeId());
  assert(not enumItem.has_value());
  NodeId id = item->getNodeId();
  enumItemsMappings[id] = {parent, item};
}

std::optional<ast::Implementation *>
TyCtx::lookupImplementation(basic::NodeId id) {
  auto it = implItemMapping.find(id);
  if (it == implItemMapping.end())
    return std::nullopt;

  return it->second;
}

std::optional<ast::AssociatedItem *>
TyCtx::lookupAssociatedItem(basic::NodeId implId) {
  assert(false);
}

void TyCtx::insertAutoderefMapping(NodeId id,
                                   std::vector<sema::Adjustment> ad) {
  assert(autoderefMappings.find(id) == autoderefMappings.end());
  autoderefMappings.emplace(id, std::move(ad));
}

void TyCtx::insertClosureCapture(basic::NodeId closureExpr,
                                 basic::NodeId capturedItem) {
  auto it = closureCaptureMappings.find(closureExpr);
  if (it == closureCaptureMappings.end()) {
    std::set<NodeId> captures;
    captures.insert(capturedItem);
    closureCaptureMappings.insert({closureExpr, captures});
  } else {
    it->second.insert(capturedItem);
  }
}

std::set<basic::NodeId> TyCtx::getCaptures(NodeId closureExpr) {
  auto it = closureCaptureMappings.find(closureExpr);
  if (it == closureCaptureMappings.end())
    return std::set<NodeId>();
  return it->second;
}

void TyCtx::generateBuiltins() {
  // unsigned integer
  u8 = std::make_unique<TyTy::UintType>(getNextNodeId(), TyTy::UintKind::U8);
  setupBuiltin("u8", u8.get());

  u16 = std::make_unique<TyTy::UintType>(getNextNodeId(), TyTy::UintKind::U16);
  setupBuiltin("u16", u16.get());

  u32 = std::make_unique<TyTy::UintType>(getNextNodeId(), TyTy::UintKind::U32);
  setupBuiltin("u32", u32.get());

  u64 = std::make_unique<TyTy::UintType>(getNextNodeId(), TyTy::UintKind::U64);
  setupBuiltin("u64", u64.get());

  u128 =
      std::make_unique<TyTy::UintType>(getNextNodeId(), TyTy::UintKind::U128);
  setupBuiltin("u128", u128.get());

  // signed integer
  i8 = std::make_unique<TyTy::IntType>(getNextNodeId(), TyTy::IntKind::I8);
  setupBuiltin("i8", i8.get());

  i16 = std::make_unique<TyTy::IntType>(getNextNodeId(), TyTy::IntKind::I16);
  setupBuiltin("i16", i16.get());

  i32 = std::make_unique<TyTy::IntType>(getNextNodeId(), TyTy::IntKind::I32);
  setupBuiltin("i32", i32.get());

  i64 = std::make_unique<TyTy::IntType>(getNextNodeId(), TyTy::IntKind::I64);
  setupBuiltin("i64", i64.get());

  i128 = std::make_unique<TyTy::IntType>(getNextNodeId(), TyTy::IntKind::I128);
  setupBuiltin("i128", i128.get());

  // float
  f32 =
      std::make_unique<TyTy::FloatType>(getNextNodeId(), TyTy::FloatKind::F32);
  setupBuiltin("f32", f32.get());

  f64 =
      std::make_unique<TyTy::FloatType>(getNextNodeId(), TyTy::FloatKind::F64);
  setupBuiltin("f64", f64.get());

  // bool
  rbool = std::make_unique<TyTy::BoolType>(getNextNodeId());
  setupBuiltin("bool", rbool.get());

  // usize and isize
  usize = std::make_unique<TyTy::USizeType>(getNextNodeId());
  setupBuiltin("usize", usize.get());

  isize = std::make_unique<TyTy::ISizeType>(getNextNodeId());
  setupBuiltin("isize", isize.get());

  // char and str
  charType = std::make_unique<TyTy::CharType>(getNextNodeId());
  setupBuiltin("char", charType.get());
  strType = std::make_unique<TyTy::StrType>(getNextNodeId());
  setupBuiltin("str", strType.get());

  never = std::make_unique<TyTy::NeverType>(getNextNodeId());
  setupBuiltin("!", never.get());

  TyTy::TupleType *unitType = TyTy::TupleType::getUnitType(getNextNodeId());

  emptyTupleType = new ast::types::TupleType(Location::getBuiltinLocation());
  builtins.push_back({"()", emptyTupleType});
  insertBuiltin(unitType->getReference(), emptyTupleType->getNodeId(),
                unitType);
  setUnitTypeNodeId(emptyTupleType->getNodeId());
}

void TyCtx::setupBuiltin(std::string_view name, TyTy::BaseType *tyty) {
  PathIdentSegment seg = {Location::getBuiltinLocation()};
  seg.setIdentifier(Identifier(name));
  types::TypePathSegment typeSeg = {Location::getBuiltinLocation()};
  typeSeg.setSegment(seg);

  TypePath *builtinType = new types::TypePath(Location::getBuiltinLocation());
  builtinType->addSegment(typeSeg);

  builtins.push_back({std::string(name), builtinType});
  insertBuiltin(tyty->getReference(), builtinType->getNodeId(), tyty);
  // FIXME
  // tyCtx->insertNodeToHir(builtinType->getNodeId(), tyty->getReference());
  insertCanonicalPath(
      builtinType->getNodeId(),
      CanonicalPath::newSegment(builtinType->getNodeId(), name));
}

} // namespace rust_compiler::tyctx
