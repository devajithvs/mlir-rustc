#pragma once

#include "Basic/Ids.h"
#include "Lexer/Identifier.h"

#include <span>
#include <string>
#include <cassert>
#include <string_view>
#include <vector>

/// https://doc.rust-lang.org/reference/paths.html#canonical-paths
/// They come from items and path-like objects
namespace rust_compiler::adt {

using namespace rust_compiler::lexer;
using namespace rust_compiler::basic;

class CanonicalPath {
  std::vector<std::pair<basic::NodeId, Identifier>> segments;
  basic::CrateNum crateNum;

public:
  static CanonicalPath newSegment(basic::NodeId id, const Identifier &path) {
    // assert(!path.empty());
    std::vector<std::pair<basic::NodeId, Identifier>> segment;
    segment.push_back({std::pair<basic::NodeId, Identifier>(id, path)});
    return CanonicalPath(segment, basic::UNKNOWN_CREATENUM);
  }

  static CanonicalPath createEmpty() {
    return CanonicalPath({}, basic::UNKNOWN_CREATENUM);
  }

  static CanonicalPath getBigSelf(basic::NodeId id) {
    return CanonicalPath::newSegment(id, Identifier("Self"));
  }

  static CanonicalPath getSmallSelf(basic::NodeId id) {
    return CanonicalPath::newSegment(id, Identifier("self"));
  }

  std::string asString() const {
    std::string buf;
    for (size_t i = 0; i < segments.size(); ++i) {
      bool haveMore = (i + 1) < segments.size();
      const std::string &seg = segments[i].second.toString();
      buf += seg + (haveMore ? "::" : "");
    }
    return buf;
  }

  CanonicalPath append(const CanonicalPath &other) const {
    assert(!other.isEmpty());
    if (isEmpty())
      return CanonicalPath(other.segments, crateNum);

    std::vector<std::pair<basic::NodeId, Identifier>> copy(segments);
    for (auto &s : other.segments)
      copy.push_back(s);

    return CanonicalPath(copy, crateNum);
  }

  basic::NodeId getNodeId() const {
    assert(!segments.empty());
    return segments.back().first;
  }

  void setCrateNum(basic::CrateNum n) { crateNum = n; }
  bool isEmpty() const { return segments.size() == 0; }

  size_t getSize() const { return segments.size(); }

  /// Note that it ignores the NodeId
  bool isEqual(const CanonicalPath &other) {
    if (other.getSize() != getSize())
      return false;
    for (unsigned i = 0; i < segments.size(); ++i)
      if (segments[i].second != other.segments[i].second)
        return false;
    return true;
  }

  static CanonicalPath
  traitImplProjectionSegment(NodeId id, const CanonicalPath &traitSegment,
                             const CanonicalPath &implTypeSegment) {
    return CanonicalPath::newSegment(
        id, Identifier("<" + implTypeSegment.asString() + " as " +
                       traitSegment.asString() + ">"));
  }

  /// it ignores the node ids
  bool isEqualByName(const CanonicalPath &b) const;

  /// Note that it ignores the NodeId
  bool operator==(const CanonicalPath &other) const {
    if (segments.size() != other.segments.size())
      return false;

    for (unsigned i = 0; i < segments.size(); ++i)
      if (segments[i].second != other.segments[i].second)
        return false;

    return true;
  }

  /// Note that it ignores the NodeId
  bool operator<(const CanonicalPath &other) const {
    if (segments.size() < other.segments.size())
      return true;
    if (segments.size() > other.segments.size())
      return false;
    // ==!
    for (unsigned i = 0; i < segments.size(); ++i)
      if (segments[i].second < other.segments[i].second)
        return true;
    return false;
  }

private:
  explicit CanonicalPath(std::vector<std::pair<basic::NodeId, Identifier>> path,
                         basic::CrateNum crateNum)
      : crateNum(crateNum) {
    for (unsigned i = 0; i < path.size(); ++i)
      segments.push_back(path[i]);
    segments.size();
  }
};

} // namespace rust_compiler::adt
