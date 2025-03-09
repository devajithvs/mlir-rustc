#ifndef TOKEN_H
#define TOKEN_H

#include <cstdint>
#include <string>
#include <optional>
#include <variant>
#include <vector>

/// `#"abc"#`, `##"a"` (fewer closing), or even `#"a` (unterminated).
///
/// Can capture fewer closing hashes than starting hashes,
/// for more efficient lexing and better backwards diagnostics.
struct GuardedStr {
    uint32_t n_hashes;
    bool terminated;
    uint32_t token_len;

    GuardedStr(uint32_t hashes, bool term, uint32_t len)
        : n_hashes(hashes), terminated(term), token_len(len) {}
};

/// Errors that can occur when parsing raw string literals.
enum class RawStrError {
    /// Non `#` characters exist between `r` and `"`, e.g. `r##~"abcde"##`
    InvalidStarter,

    /// The string was not terminated, e.g. `r###"abcde"##`.
    /// `possible_terminator_offset` is the number of characters after `r` or
    /// `br` where they may have intended to terminate it.
    NoTerminator,

    /// More than 255 `#`s exist.
    TooManyDelimiters
};

/// Struct to hold error details for raw string parsing.
struct RawStrErrorDetails {
    RawStrError error;
    char bad_char;  // Only for `InvalidStarter`
    uint32_t expected;
    uint32_t found;
    std::optional<uint32_t> possible_terminator_offset;

    RawStrErrorDetails(RawStrError err, char bad = '\0', uint32_t exp = 0, uint32_t fnd = 0,
                       std::optional<uint32_t> offset = std::nullopt)
        : error(err), bad_char(bad), expected(exp), found(fnd), possible_terminator_offset(offset) {}
};

/// Base of numeric literal encoding according to its prefix.
enum class Base : uint8_t {
    Binary = 2,      // Literal starts with "0b"
    Octal = 8,       // Literal starts with "0o"
    Decimal = 10,    // No prefix
    Hexadecimal = 16 // Literal starts with "0x"
};

/// Documentation style for doc comments.
enum class DocStyle { Outer, Inner };

/// Enum representing the literal types supported by the lexer.
struct LiteralKind {
    enum class Type { Int, Float, Char, Byte, Str, ByteStr, CStr, RawStr, RawByteStr, RawCStr };
    
    Type kind;
    Base base;
    bool empty_int = false;
    bool empty_exponent = false;
    std::optional<uint8_t> n_hashes;
    bool terminated = false;

    // Factory methods for constructing literal types
    static LiteralKind IntLiteral(Base base, bool empty_int) {
        return { Type::Int, base, empty_int, false, std::nullopt, false };
    }

    static LiteralKind FloatLiteral(Base base, bool empty_exponent) {
        return { Type::Float, base, false, empty_exponent, std::nullopt, false };
    }

    static LiteralKind CharLiteral(bool terminated) {
        return { Type::Char, Base::Decimal, false, false, std::nullopt, terminated };
    }

    static LiteralKind ByteLiteral(bool terminated) {
        return { Type::Byte, Base::Decimal, false, false, std::nullopt, terminated };
    }

    static LiteralKind StrLiteral(bool terminated) {
        return { Type::Str, Base::Decimal, false, false, std::nullopt, terminated };
    }

    static LiteralKind ByteStrLiteral(bool terminated) {
        return { Type::ByteStr, Base::Decimal, false, false, std::nullopt, terminated };
    }

    static LiteralKind CStrLiteral(bool terminated) {
        return { Type::CStr, Base::Decimal, false, false, std::nullopt, terminated };
    }

    static LiteralKind RawStrLiteral(std::optional<uint8_t> n_hashes) {
        return { Type::RawStr, Base::Decimal, false, false, n_hashes, false };
    }

    static LiteralKind RawByteStrLiteral(std::optional<uint8_t> n_hashes) {
        return { Type::RawByteStr, Base::Decimal, false, false, n_hashes, false };
    }

    static LiteralKind RawCStrLiteral(std::optional<uint8_t> n_hashes) {
        return { Type::RawCStr, Base::Decimal, false, false, n_hashes, false };
    }
};

/// Enum representing different token types.
enum class TokenKind {
    LineComment, // `// comment`
    BlockComment, // `/* comment */`
    Whitespace, // Spaces, tabs, etc.
    Ident, // Identifier or keyword
    InvalidIdent, // Invalid identifier (contains emoji, etc.)
    RawIdent, // `r#ident`
    UnknownPrefix, // Unknown prefix `foo#`
    UnknownPrefixLifetime, // Unknown lifetime prefix `'foo#`
    RawLifetime, // `'r#foo`
    GuardedStrPrefix, // `#"` or `##` (Rust 2024 feature)
    Literal, // General literals
    Lifetime, // `'a`
    
    // Punctuation tokens
    Semi, Comma, Dot, OpenParen, CloseParen, OpenBrace, CloseBrace,
    OpenBracket, CloseBracket, At, Pound, Tilde, Question, Colon,
    Dollar, Eq, Bang, Lt, Gt, Minus, And, Or, Plus, Star, Slash, Caret, Percent,

    // Unknown or End of File
    Unknown, 
    Eof 
};

/// Token struct storing information about parsed tokens.
class Token {
public:
    TokenKind kind;
    uint32_t len;
    std::optional<LiteralKind> literal_kind; // Stores literal information if applicable

    /// Constructor for non-literal tokens
    Token(TokenKind kind, uint32_t len) : kind(kind), len(len) {}
    
    /// Constructor for literal tokens
    Token(TokenKind kind, LiteralKind lit_kind, uint32_t len)
        : kind(kind), len(len), literal_kind(lit_kind) {}

    /// Factory method for creating a new token
    static Token newToken(TokenKind kind, uint32_t len) {
        return Token(kind, len);
    }
};

#endif // TOKEN_H
