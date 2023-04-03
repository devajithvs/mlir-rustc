#pragma once

#include "Lexer/TokenStream.h"
#include "unicode/uchar.h"

#include <string_view>

namespace rust_compiler::lexer {

TokenStream lex(std::string_view code, std::string_view fileName);

/// https://doc.rust-lang.org/reference/tokens.html
class Lexer {
  std::string chars;
  std::string fileName;
  uint32_t remaining;
  TokenStream tokenStream;
  uint32_t offset;

public:
  void lex();

private:
  std::optional<char> bump();

  Token advanceToken();

  Token lexChar();
  Token lexString();
  Token lexRawString();
  Token lexByte();
  Token lexByteString();
  Token lexRawByte();
  Token lexRawByteString();
  Token lexStringLiteral();
  Token lexRawDoubleQuotedString();
  Token lexRawIdentifier();

  Token lexIntegerLiteral();
  Token lexFloatLiteral();

  Token lexLifetimeToken();
  Token lexLifetimeOrLabel();
  Token lexLifetimeOrChar();

  Token lexNumericalLiteral();
  Token lexBinLiteral();
  Token lexOctLiteral();
  Token lexHexLiteral();
  Token lexDecOrFloatLiteral();

  Token lexIdentifierOrKeyWord();
  Token lexIdentifierOrUnknownPrefix();
  Token lexFakeIdentifierOrUnknownPrefix();

  Location getLocation();

  void lineComment();
  void blockComment();

  bool isWhiteSpace();
  void skipWhiteSpace();

  bool isASCII();
  bool isEmoji();

  bool isIdStart(int i = 0);
  bool isIdContinue(int i = 0);
  UChar32 getUchar(int i = 0);
  void skip();
  char peek(int i = 0);
};

} // namespace rust_compiler::lexer
