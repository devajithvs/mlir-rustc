//! Low-level Rust lexer.
//!
//! The idea with `rustc_lexer` is to make a reusable library,
//! by separating out pure lexing and rustc-specific concerns, like spans,
//! error reporting, and interning. So, rustc_lexer operates directly on `&str`,
//! produces simple tokens which are a pair of type-tag and a bit of original text,
//! and does not report errors, instead storing them as flags on the token.
//!
//! Tokens produced by this lexer are not yet ready for parsing the Rust syntax.
//! For that see [`rustc_parse::lexer`], which converts this basic token stream
//! into wide tokens used by actual parser.
//!
//! The purpose of this crate is to convert raw sources into a labeled sequence
//! of well-known token types, so building an actual Rust token stream will
//! be easier.
//!
//! The main entity of this crate is the [`TokenKind`] enum which represents common
//! lexeme types.
//!
//! [`rustc_parse::lexer`]: ../rustc_parse/lexer/index.html

// tidy-alphabetical-start
// We want to be able to build this crate with a stable compiler,
// so no `#![feature]` attributes should be added.

#include "Lexer.h"
#include "Cursor.h"
#include "Unescape.h"

#include <cassert>

#define EOF_CHAR '\0'

/// `rustc` allows files to have a shebang, e.g. "#!/usr/bin/rustrun",
/// but shebang isn't a part of rust syntax.
std::optional<size_t> strip_shebang(const std::string& input) {
    // Shebang must start with `#!` literally, without any preceding whitespace.
    // For simplicity we consider any line starting with `#!` a shebang,
    // regardless of restrictions put on shebangs by specific platforms.
    // FIXME: TODO
    if (input.rfind("#!", 0) == 0) {
        size_t newline_pos = input.find('\n');
        if (newline_pos != std::string::npos) {
            return newline_pos + 1;
        }
        return input.size();  // Whole input is a shebang
    }
    return std::nullopt;
}

/// Validates a raw string literal. Used for getting more information about a
/// problem with a `RawStr`/`RawByteStr` with a `std::nullopt` field.
std::variant<std::monostate, RawStrErrorDetails> validate_raw_str(const std::string& input, uint32_t prefix_len) {
    assert(!input.empty());
    Cursor cursor(input);
    for (uint32_t i = 0; i < prefix_len; ++i) {
        if (!cursor.is_eof()) cursor.bump();
    }

    uint32_t n_hashes = 0;
    while (!cursor.is_eof() && cursor.first() == '#') {
        ++n_hashes;
        cursor.bump();
    }

    if (cursor.first() != '"') {
        return RawStrErrorDetails(RawStrError::InvalidStarter, cursor.first());
    }
    cursor.bump();

    while (!cursor.is_eof() && cursor.first() != '"') {
        cursor.bump();
    }

    if (cursor.is_eof()) {
        return RawStrErrorDetails(RawStrError::NoTerminator, '\0', n_hashes, 0);
    }

    cursor.bump();
    return std::monostate{};
}

std::vector<Token> Lexer::tokenize() {
    while (!cursor.is_eof()) {
        TokenKind kind = advance_token();
        tokens.emplace_back(kind, 1);  // Placeholder for len, update accordingly
    }
    tokens.emplace_back(TokenKind::Eof, 0);
    return tokens;
}

TokenKind Lexer::advance_token() {
    char first_char = cursor.first();
    if (first_char == '\0') return TokenKind::Eof;

    if (std::isspace(first_char)) return lex_whitespace();
    if (first_char == '/') return line_comment();
    if (std::isalpha(first_char) || first_char == '_') return ident_or_unknown_prefix();
    if (std::isdigit(first_char)) return lex_number();
    return lex_punctuation();
}

TokenKind Lexer::lex_whitespace() {
    cursor.eat_while([](char c) { return std::isspace(c); });
    return TokenKind::Whitespace;
}

TokenKind Lexer::line_comment() {
    cursor.bump();
    if (cursor.first() == '/') {
        cursor.eat_until('\n');
        return TokenKind::LineComment;
    } else if (cursor.first() == '*') {
        cursor.bump();
        while (!cursor.is_eof()) {
            if (cursor.first() == '*' && cursor.second() == '/') {
                cursor.bump();
                cursor.bump();
                return TokenKind::BlockComment;
            }
            cursor.bump();
        }
        return TokenKind::BlockComment; // Unterminated block comment
    }
    return TokenKind::Slash;
}

TokenKind Lexer::ident_or_unknown_prefix() {
    cursor.eat_while([](char c) { return std::isalnum(c) || c == '_'; });
    return TokenKind::Ident;
}

TokenKind Lexer::lex_number() {
    cursor.eat_while([](char c) { return std::isdigit(c); });
    return TokenKind::Literal;
}

TokenKind Lexer::lex_punctuation() {
    char c = cursor.first();
    cursor.bump();

    switch (c) {
        case ';': return TokenKind::Semi;
        case ',': return TokenKind::Comma;
        case '.': return TokenKind::Dot;
        case '(': return TokenKind::OpenParen;
        case ')': return TokenKind::CloseParen;
        case '{': return TokenKind::OpenBrace;
        case '}': return TokenKind::CloseBrace;
        case '[': return TokenKind::OpenBracket;
        case ']': return TokenKind::CloseBracket;
        case '@': return TokenKind::At;
        case '#': return TokenKind::Pound;
        case '~': return TokenKind::Tilde;
        case '?': return TokenKind::Question;
        case ':': return TokenKind::Colon;
        case '$': return TokenKind::Dollar;
        case '=': return TokenKind::Eq;
        case '!': return TokenKind::Bang;
        case '<': return TokenKind::Lt;
        case '>': return TokenKind::Gt;
        case '-': return TokenKind::Minus;
        case '&': return TokenKind::And;
        case '|': return TokenKind::Or;
        case '+': return TokenKind::Plus;
        case '*': return TokenKind::Star;
        case '/': return TokenKind::Slash;
        case '^': return TokenKind::Caret;
        case '%': return TokenKind::Percent;
        default: return TokenKind::Unknown;
    }
}

// Parses a character literal or a lifetime
TokenKind Lexer::lifetime_or_char() {
    assert(cursor.prev() == '\'');

    bool can_be_a_lifetime = true;
    
    if (cursor.second() == '\'') {
        // It's surely not a lifetime.
        can_be_a_lifetime = false;
    } else {
        // If the first symbol is valid for an identifier, it can be a lifetime.
        // Also check if it's a number for better error reporting (so '0 will
        // be reported as an invalid lifetime and not as an unterminated char literal).
        can_be_a_lifetime = is_id_start(cursor.first()) || std::isdigit(cursor.first());
    }

    if (!can_be_a_lifetime) {
        bool terminated = single_quoted_string();
        uint32_t suffix_start = cursor.pos_within_token();
        if (terminated) {
            eat_literal_suffix();
        }
        // FIXME: Might have a problem here
        return TokenKind::Literal;
    }

    if (cursor.first() == 'r' && cursor.second() == '#' && is_id_start(cursor.third())) {
        // Eat "r" and `#`, and identifier start characters.
        cursor.bump();
        cursor.bump();
        cursor.bump();
        cursor.eat_while([this](char c) { return is_id_continue(c); });
        return TokenKind::RawLifetime;
    }

    // Either a lifetime or a character literal with length greater than 1.
    bool starts_with_number = std::isdigit(cursor.first());

    // Skip the literal contents.
    cursor.bump(); // First symbol can be a number, so skip it.
    cursor.eat_while([this](char c) { return is_id_continue(c); });

    switch (cursor.first()) {
        case '\'':
            cursor.bump();
            // FIXME: Might have a problem here
            return TokenKind::Literal;
        case '#':
            if (!starts_with_number) {
                return TokenKind::UnknownPrefixLifetime;
            }
            [[fallthrough]];
        default:
            return TokenKind::Lifetime;
    }
}

// Parses a single-quoted character literal
bool Lexer::single_quoted_string() {
    assert(cursor.prev() == '\'');

    // Check if it's a one-symbol literal.
    if (cursor.second() == '\'' && cursor.first() != '\\') {
        cursor.bump();
        cursor.bump();
        return true;
    }

    // Literal has more than one symbol.
    while (!cursor.is_eof()) {
        switch (cursor.first()) {
            case '\'':
                cursor.bump();
                return true;
            case '/': // Avoiding comment confusion
                return false;
            case '\n':
                if (cursor.second() != '\'') {
                    return false;
                }
                break;
            case EOF_CHAR:
                return false;
            case '\\': // Handle escaped characters
                cursor.bump();
                cursor.bump();
                break;
            default:
                cursor.bump();
        }
    }
    return false; // Unterminated string
}

// Parses a double-quoted string literal
bool Lexer::double_quoted_string() {
    assert(cursor.prev() == '"');

    while (!cursor.is_eof()) {
        char next_char = cursor.bump().value_or(EOF_CHAR);
        switch (next_char) {
            case '"':
                return true;
            case '\\':
                if (cursor.first() == '\\' || cursor.first() == '"') {
                    cursor.bump(); // Consume escaped character
                }
                break;
        }
    }
    return false; // Unterminated string
}

// Parses a guarded string literal with `#"` or `##"` (reserved syntax for future Rust editions)
std::optional<GuardedStr> Lexer::guarded_double_quoted_string() {
    assert(cursor.prev() != '#');

    uint32_t n_start_hashes = 0;
    while (cursor.first() == '#') {
        n_start_hashes++;
        cursor.bump();
    }

    if (cursor.first() != '"') {
        return std::nullopt;
    }
    cursor.bump();
    assert(cursor.prev() == '"');

    bool terminated = double_quoted_string();
    uint32_t token_len = cursor.pos_within_token();
    cursor.reset_pos_within_token();

    if (!terminated) {
        return GuardedStr{n_start_hashes, false, token_len};
    }

    uint32_t n_end_hashes = 0;
    while (cursor.first() == '#' && n_end_hashes < n_start_hashes) {
        n_end_hashes++;
        cursor.bump();
    }

    eat_literal_suffix();
    return GuardedStr{n_start_hashes, true, token_len};
}

std::optional<uint8_t> Lexer::raw_double_quoted_string(uint32_t prefix_len) {
    // Wrap the actual function to handle the error with too many hashes.
    // This way, it eats the whole raw string.
    auto n_hashes = raw_string_unvalidated(prefix_len);

    // Only up to 255 `#`s are allowed in raw strings
    if (n_hashes > 255) {
        return std::nullopt; // Indicate error: Too many delimiters
    }

    return static_cast<uint8_t>(n_hashes); // Return the valid hash count
}

// // Parses a raw double-quoted string literal
// std::variant<uint8_t, RawStrError> Lexer::raw_double_quoted_string(uint32_t prefix_len) {
//     uint32_t n_hashes = raw_string_unvalidated(prefix_len);
//     if (n_hashes > 255) {
//         return RawStrError::TooManyDelimiters;
//     }
//     return static_cast<uint8_t>(n_hashes);
// }

// Internal function to validate and parse raw strings
uint32_t Lexer::raw_string_unvalidated(uint32_t prefix_len) {
    assert(cursor.prev() == 'r');
    uint32_t start_pos = cursor.pos_within_token();
    std::optional<uint32_t> possible_terminator_offset;
    uint32_t max_hashes = 0;

    uint32_t n_start_hashes = 0;
    while (cursor.first() == '#') {
        n_start_hashes++;
        cursor.bump();
    }

    if (cursor.bump() != '"') {
        return 0; // Invalid starter
    }

    while (!cursor.is_eof()) {
        cursor.eat_until('"');

        if (cursor.is_eof()) {
            return 0;
        }

        cursor.bump(); // Consume closing `"`

        uint32_t n_end_hashes = 0;
        while (cursor.first() == '#' && n_end_hashes < n_start_hashes) {
            n_end_hashes++;
            cursor.bump();
        }

        if (n_end_hashes == n_start_hashes) {
            return n_start_hashes;
        } else if (n_end_hashes > max_hashes) {
            possible_terminator_offset = cursor.pos_within_token() - start_pos - n_end_hashes + prefix_len;
            max_hashes = n_end_hashes;
        }
    }
    return 0;
}

// Helper function to eat decimal digits
bool Lexer::eat_decimal_digits() {
    bool has_digits = false;
    while (true) {
        switch (cursor.first()) {
            case '_':
                cursor.bump();
                break;
            case '0' ... '9':
                has_digits = true;
                cursor.bump();
                break;
            default:
                return has_digits;
        }
    }
}

// Helper function to eat hexadecimal digits
bool Lexer::eat_hexadecimal_digits() {
    bool has_digits = false;
    while (true) {
        switch (cursor.first()) {
            case '_':
                cursor.bump();
                break;
            case '0' ... '9':
            case 'a' ... 'f':
            case 'A' ... 'F':
                has_digits = true;
                cursor.bump();
                break;
            default:
                return has_digits;
        }
    }
}

// Parses a float exponent
bool Lexer::eat_float_exponent() {
    assert(cursor.prev() == 'e' || cursor.prev() == 'E');

    if (cursor.first() == '-' || cursor.first() == '+') {
        cursor.bump();
    }
    return eat_decimal_digits();
}

// Eats the suffix of a literal (e.g., `u8`)
void Lexer::eat_literal_suffix() {
    eat_identifier();
}

// Consumes an identifier
void Lexer::eat_identifier() {
    if (!is_id_start(cursor.first())) {
        return;
    }
    cursor.bump();
    cursor.eat_while([this](char c) { return is_id_continue(c); });
}

// Checks if a character is a valid identifier start
bool Lexer::is_id_start(char c) {
    return std::isalpha(c) || c == '_';
}

// Checks if a character can continue an identifier
bool Lexer::is_id_continue(char c) {
    return std::isalnum(c) || c == '_';
}
