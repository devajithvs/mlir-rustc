#ifndef TOKEN_H
#define TOKEN_H

#include <cstdint>
#include <string>
#include <optional>

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
    /// Literal starts with "0b".
    Binary = 2,

    /// Literal starts with "0o".
    Octal = 8,

    /// Literal doesn't contain a prefix.
    Decimal = 10,

    /// Literal starts with "0x".
    Hexadecimal = 16
};

enum class DocStyle { Outer, Inner };

enum class TokenKind {
    LineComment,
    BlockComment,
    Whitespace,
    Ident,
    InvalidIdent,
    RawIdent,
    UnknownPrefix,
    UnknownPrefixLifetime,
    RawLifetime,
    GuardedStrPrefix,
    Literal,
    Lifetime,
    Semi,
    Comma,
    Dot,
    OpenParen,
    CloseParen,
    OpenBrace,
    CloseBrace,
    OpenBracket,
    CloseBracket,
    At,
    Pound,
    Tilde,
    Question,
    Colon,
    Dollar,
    Eq,
    Bang,
    Lt,
    Gt,
    Minus,
    And,
    Or,
    Plus,
    Star,
    Slash,
    Caret,
    Percent,
    Unknown,
    Eof
};

struct LiteralKind {
    enum class Type { Int, Float, Char, Byte, Str, ByteStr, CStr, RawStr, RawByteStr, RawCStr };
    
    Type kind;
    Base base;
    bool empty_int = false;
    bool empty_exponent = false;
    std::optional<int> n_hashes;
    bool terminated = false;

    static LiteralKind IntLiteral(Base base, bool empty_int) {
        return { Type::Int, base, empty_int };
    }

    static LiteralKind FloatLiteral(Base base, bool empty_exponent) {
        return { Type::Float, base, false, empty_exponent };
    }
};

class Token {
public:
    TokenKind kind;
    uint32_t len;

    Token(TokenKind kind, uint32_t len) : kind(kind), len(len) {}

    static Token newToken(TokenKind kind, uint32_t len) {
        return Token(kind, len);
    }
};

#endif // TOKEN_H
