#include "AST/Patterns/GroupedPattern.h"
#include "AST/Patterns/LiteralPattern.h"
#include "AST/Patterns/RangePattern.h"
#include "AST/Patterns/RestPattern.h"
#include "AST/Patterns/SlicePattern.h"
#include "AST/Patterns/TuplePattern.h"
#include "Lexer/Token.h"
#include "Parser/Parser.h"

#include <llvm/Support/raw_ostream.h>

using namespace rust_compiler::ast;
using namespace rust_compiler::adt;
using namespace llvm;
using namespace rust_compiler::lexer;
using namespace rust_compiler::ast::patterns;

namespace rust_compiler::parser {

StringResult<std::shared_ptr<ast::patterns::PatternNoTopAlt>>
Parser::parseRestPattern() {
  Location loc = getLocation();

  RestPattern pattern = {loc};

  return StringResult<std::shared_ptr<ast::patterns::PatternNoTopAlt>>(
      std::make_shared<RestPattern>(pattern));
}

StringResult<std::shared_ptr<ast::patterns::PatternNoTopAlt>>
Parser::parseIdentifierPattern() {
  Location loc = getLocation();

  IdentifierPattern pattern = {loc};

  if (checkKeyWord(KeyWordKind::KW_REF)) {
    assert(eatKeyWord(KeyWordKind::KW_REF));
    pattern.setRef();
  }

  if (checkKeyWord(KeyWordKind::KW_MUT)) {
    assert(eatKeyWord(KeyWordKind::KW_MUT));
    pattern.setMut();
  }

  if (!check(TokenKind::Identifier)) {
    // report errro
  }

  Token tok = getToken();
  pattern.setIdentifier(tok.getIdentifier());
  assert(eat(TokenKind::Identifier));

  if (check(TokenKind::At)) {
    assert(eat(TokenKind::At));

    StringResult<std::shared_ptr<ast::patterns::PatternNoTopAlt>>
        patternNoTopAlt = parsePatternNoTopAlt();
    if (!patternNoTopAlt) {
      llvm::errs()
          << "failed to parse pattern to top alt in identifier pattern: "
          << patternNoTopAlt.getError() << "\n";
      printFunctionStack();
      exit(EXIT_FAILURE);
    }
    pattern.addPattern(patternNoTopAlt.getValue());
    return StringResult<std::shared_ptr<ast::patterns::PatternNoTopAlt>>(
        std::make_shared<IdentifierPattern>(pattern));
  }

  return StringResult<std::shared_ptr<ast::patterns::PatternNoTopAlt>>(
      std::make_shared<IdentifierPattern>(pattern));
}

StringResult<std::shared_ptr<ast::patterns::PatternNoTopAlt>>
Parser::parseLiteralPattern() {
  Location loc = getLocation();

  LiteralPattern pattern = {loc};

  if (checkKeyWord(KeyWordKind::KW_TRUE)) {
    pattern.setKind(LiteralPatternKind::True, getToken().getStorage());
    assert(eatKeyWord(KeyWordKind::KW_TRUE));
    return StringResult<std::shared_ptr<ast::patterns::PatternNoTopAlt>>(
        std::make_shared<LiteralPattern>(pattern));
  } else if (checkKeyWord(KeyWordKind::KW_FALSE)) {
    pattern.setKind(LiteralPatternKind::False, getToken().getStorage());
    assert(eatKeyWord(KeyWordKind::KW_FALSE));
    return StringResult<std::shared_ptr<ast::patterns::PatternNoTopAlt>>(
        std::make_shared<LiteralPattern>(pattern));
  } else if (check(TokenKind::CHAR_LITERAL)) {
    pattern.setKind(LiteralPatternKind::CharLiteral, getToken().getStorage());
    assert(eat(TokenKind::CHAR_LITERAL));
    return StringResult<std::shared_ptr<ast::patterns::PatternNoTopAlt>>(
        std::make_shared<LiteralPattern>(pattern));
  } else if (check(TokenKind::BYTE_LITERAL)) {
    pattern.setKind(LiteralPatternKind::ByteLiteral, getToken().getStorage());
    assert(eat(TokenKind::BYTE_LITERAL));
    return StringResult<std::shared_ptr<ast::patterns::PatternNoTopAlt>>(
        std::make_shared<LiteralPattern>(pattern));
  } else if (check(TokenKind::STRING_LITERAL)) {
    pattern.setKind(LiteralPatternKind::StringLiteral, getToken().getStorage());
    assert(eat(TokenKind::STRING_LITERAL));
    return StringResult<std::shared_ptr<ast::patterns::PatternNoTopAlt>>(
        std::make_shared<LiteralPattern>(pattern));
  } else if (check(TokenKind::RAW_STRING_LITERAL)) {
    pattern.setKind(LiteralPatternKind::RawStringLiteral,
                    getToken().getStorage());
    assert(eat(TokenKind::RAW_STRING_LITERAL));
    return StringResult<std::shared_ptr<ast::patterns::PatternNoTopAlt>>(
        std::make_shared<LiteralPattern>(pattern));
  } else if (check(TokenKind::BYTE_STRING_LITERAL)) {
    pattern.setKind(LiteralPatternKind::ByteStringLiteral,
                    getToken().getStorage());
    assert(eat(TokenKind::BYTE_STRING_LITERAL));
    return StringResult<std::shared_ptr<ast::patterns::PatternNoTopAlt>>(
        std::make_shared<LiteralPattern>(pattern));
  } else if (check(TokenKind::RAW_BYTE_STRING_LITERAL)) {
    pattern.setKind(LiteralPatternKind::RawByteStringLiteral,
                    getToken().getStorage());
    assert(eat(TokenKind::RAW_BYTE_STRING_LITERAL));
    return StringResult<std::shared_ptr<ast::patterns::PatternNoTopAlt>>(
        std::make_shared<LiteralPattern>(pattern));
  } else if (check(TokenKind::INTEGER_LITERAL)) {
    pattern.setKind(LiteralPatternKind::IntegerLiteral,
                    getToken().getStorage());
    assert(eat(TokenKind::INTEGER_LITERAL));
    return StringResult<std::shared_ptr<ast::patterns::PatternNoTopAlt>>(
        std::make_shared<LiteralPattern>(pattern));
  } else if (check(TokenKind::FLOAT_LITERAL)) {
    pattern.setKind(LiteralPatternKind::FloatLiteral, getToken().getStorage());
    assert(eat(TokenKind::FLOAT_LITERAL));
    return StringResult<std::shared_ptr<ast::patterns::PatternNoTopAlt>>(
        std::make_shared<LiteralPattern>(pattern));
  } else if (check(TokenKind::Minus) && check(TokenKind::INTEGER_LITERAL, 1)) {
    pattern.setKind(LiteralPatternKind::IntegerLiteral,
                    getToken().getStorage());
    pattern.setLeadingMinus();
    assert(eat(TokenKind::Minus));
    assert(eat(TokenKind::INTEGER_LITERAL));
    return StringResult<std::shared_ptr<ast::patterns::PatternNoTopAlt>>(
        std::make_shared<LiteralPattern>(pattern));
  } else if (check(TokenKind::Minus) && check(TokenKind::FLOAT_LITERAL, 1)) {
    pattern.setKind(LiteralPatternKind::FloatLiteral, getToken().getStorage());
    pattern.setLeadingMinus();
    assert(eat(TokenKind::Minus));
    assert(eat(TokenKind::FLOAT_LITERAL));
    return StringResult<std::shared_ptr<ast::patterns::PatternNoTopAlt>>(
        std::make_shared<LiteralPattern>(pattern));
  }
  return StringResult<std::shared_ptr<ast::patterns::PatternNoTopAlt>>(
      "failed to parse literal pattern");
}

StringResult<std::shared_ptr<ast::patterns::PatternNoTopAlt>>
Parser::parseRangePattern() {
  Location loc = getLocation();

  RangePattern pattern = {loc};

  assert(false);
}

StringResult<std::shared_ptr<ast::patterns::Pattern>> Parser::parsePattern() {
  Location loc = getLocation();

  Pattern pattern = {loc};

  if (check(TokenKind::Or)) {
    assert(check(TokenKind::Or));
    pattern.setLeadingOr();
  }

  StringResult<std::shared_ptr<ast::patterns::PatternNoTopAlt>> first =
      parsePatternNoTopAlt();
  if (!first) {
    llvm::errs() << "failed to parse pattern to top alt in pattern: "
                 << first.getError() << "\n";
    printFunctionStack();
    exit(EXIT_FAILURE);
  }
  pattern.addPattern(first.getValue());

  if (check(TokenKind::Or)) {
    assert(check(TokenKind::Or));

    while (true) {
      StringResult<std::shared_ptr<ast::patterns::PatternNoTopAlt>>
          patternNoTopAlt = parsePatternNoTopAlt();
      if (!patternNoTopAlt) {
        llvm::errs() << "failed to parse pattern to top alt in pattern: "
                     << patternNoTopAlt.getError() << "\n";
        printFunctionStack();
        exit(EXIT_FAILURE);
      }
      pattern.addPattern(patternNoTopAlt.getValue());
      if (check(TokenKind::Or)) {
        assert(check(TokenKind::Or));
        continue;
      } else if (check(TokenKind::Eof)) {
        return StringResult<std::shared_ptr<ast::patterns::Pattern>>(
            "found eof in  pattern");

      } else {
        return StringResult<std::shared_ptr<ast::patterns::Pattern>>(
            std::make_shared<Pattern>(pattern));
      }
    }
  }
  return StringResult<std::shared_ptr<ast::patterns::Pattern>>(
      std::make_shared<Pattern>(pattern));
}

StringResult<std::shared_ptr<ast::patterns::PatternNoTopAlt>>
Parser::parseTupleOrGroupedPattern() {
  Location loc = getLocation();

  if (!check(TokenKind::ParenOpen)) {
    llvm::errs() << "failed to parse token ( in tuple or group pattern "
                 << "\n";
    // report error
    return StringResult<std::shared_ptr<ast::patterns::PatternNoTopAlt>>(
        "failed to parse (");
  }
  assert(check(TokenKind::ParenOpen));

  if (check(TokenKind::DotDot) && check(TokenKind::ParenClose, 1)) {
    TuplePattern tuple = {loc};

    assert(eat(TokenKind::DotDot));
    assert(eat(TokenKind::ParenClose));

    TuplePatternItems items = {loc};
    items.setRestPattern();
    tuple.setItems(items);

    return StringResult<std::shared_ptr<ast::patterns::PatternNoTopAlt>>(
        std::make_shared<TuplePattern>(tuple));
  }

  StringResult<std::shared_ptr<ast::patterns::Pattern>> pattern =
      parsePattern();
  if (!pattern) {
    llvm::errs() << "failed to parse pattern in pattern: " << pattern.getError()
                 << "\n";
    printFunctionStack();
    return StringResult<std::shared_ptr<ast::patterns::PatternNoTopAlt>>(
        "failed to parse pattern in tuple or group pattern");
    // exit(EXIT_FAILURE);
  }
  if (check(TokenKind::ParenClose)) {
    assert(check(TokenKind::ParenClose));
    // done GroupedPattern
    GroupedPattern group = {loc};
    group.setPattern(pattern.getValue());
    return StringResult<std::shared_ptr<ast::patterns::PatternNoTopAlt>>(
        std::make_shared<GroupedPattern>(group));
  } else if (check(TokenKind::Comma) && check(TokenKind::ParenClose, 1)) {
    TuplePatternItems items = {loc};

    items.addPattern(pattern.getValue());
    items.setTrailingComma();

    TuplePattern tuple = {loc};
    tuple.setItems(items);

    return StringResult<std::shared_ptr<ast::patterns::PatternNoTopAlt>>(
        std::make_shared<TuplePattern>(tuple));
  } else if (check(TokenKind::Comma) && !check(TokenKind::ParenClose, 1)) {
    // continue
  } else {
    // report error ?
    llvm::errs() << "found unexpected token in tuple or grouped pattern"
                 << Token2String(getToken().getKind()) << "\n";
    return StringResult<std::shared_ptr<ast::patterns::PatternNoTopAlt>>(
        "failed to parse pattern in tuple or group pattern: unexpected token");
  }

  TuplePatternItems items = {loc};

  items.addPattern(pattern.getValue());

  while (true) {
    StringResult<std::shared_ptr<ast::patterns::Pattern>> pattern =
        parsePattern();
    if (!pattern) {
      llvm::errs() << "failed to parse pattern in turple or grouped pattern: "
                   << pattern.getError() << "\n";
      printFunctionStack();
      return StringResult<std::shared_ptr<ast::patterns::PatternNoTopAlt>>(
        "failed to parse pattern in tuple or group pattern");
      //exit(EXIT_FAILURE);
    }
    items.addPattern(pattern.getValue());

    if (check(TokenKind::ParenClose)) {
      // done
      TuplePattern pattern = {loc};
      pattern.setItems(items);
      return StringResult<std::shared_ptr<ast::patterns::PatternNoTopAlt>>(
          std::make_shared<TuplePattern>(pattern));
    } else if (check(TokenKind::Comma) && check(TokenKind::ParenClose, 1)) {
      assert(check(TokenKind::Comma));
      TuplePattern pattern = {loc};
      pattern.setItems(items);
      items.setTrailingComma();
      return StringResult<std::shared_ptr<ast::patterns::PatternNoTopAlt>>(
          std::make_shared<TuplePattern>(pattern));
    } else if (check(TokenKind::Comma)) {
      assert(check(TokenKind::Comma));
      continue;
    } else if (check(TokenKind::Eof)) {
      // abort
    }
  }
}

StringResult<ast::patterns::SlicePatternItems>
Parser::parseSlicePatternItems() {
  Location loc = getLocation();

  SlicePatternItems items = {loc};

  StringResult<std::shared_ptr<ast::patterns::Pattern>> first = parsePattern();
  if (!first) {
    llvm::errs() << "failed to parse pattern in slice pattern items: "
                 << first.getError() << "\n";
    printFunctionStack();
    exit(EXIT_FAILURE);
  }
  items.addPattern(first.getValue());

  // TODO

  assert(false);
}

StringResult<std::shared_ptr<ast::patterns::PatternNoTopAlt>>
Parser::parseSlicePattern() {
  Location loc = getLocation();

  SlicePattern slice = {loc};

  if (!check(TokenKind::SquareOpen)) {
    return StringResult<std::shared_ptr<ast::patterns::PatternNoTopAlt>>(
        "failed to parse [ token in slice pattern");
  }
  assert(check(TokenKind::SquareOpen));

  if (check(TokenKind::SquareClose)) {
    // done
    assert(check(TokenKind::SquareClose));
    return StringResult<std::shared_ptr<ast::patterns::PatternNoTopAlt>>(
        std::make_shared<SlicePattern>(slice));
  }

  StringResult<ast::patterns::SlicePatternItems> items =
      parseSlicePatternItems();
  if (!items) {
    llvm::errs() << "failed to parse slice pattern items in slice pattern: "
                 << items.getError() << "\n";
    printFunctionStack();
    exit(EXIT_FAILURE);
  }
  slice.setPatternItems(items.getValue());

  assert(check(TokenKind::SquareClose));

  return StringResult<std::shared_ptr<ast::patterns::PatternNoTopAlt>>(
      std::make_shared<SlicePattern>(slice));
}

} // namespace rust_compiler::parser
