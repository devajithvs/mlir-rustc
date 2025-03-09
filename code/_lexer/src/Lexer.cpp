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
    // Move past the leading `r` or `br`.
    for (uint32_t i = 0; i < prefix_len; ++i) {
        if (!cursor.is_eof()) cursor.bump();
    }

    auto result = cursor.raw_double_quoted_string(prefix_len);

    // If result contains an error, return it as a RawStrErrorDetails
    if (std::holds_alternative<RawStrErrorDetails>(result)) {
        return std::get<RawStrErrorDetails>(result);
    }

    // Otherwise, return `std::monostate` (equivalent to `Ok(())` in Rust)
    return std::monostate{};
}

/// Creates an iterator that produces tokens from the input string.
std::vector<Token> tokenize(const std::string& input) {
    std::vector<Token> tokens;
    Cursor cursor(input);

    while (true) {
        Token token = cursor.advance_token();
        if (token.kind == TokenKind::Eof) {
            break;
        }
        tokens.push_back(token);
    }

    return tokens;
}

/// True if `c` is considered a whitespace according to Rust language definition.
/// See [Rust language reference](https://doc.rust-lang.org/reference/whitespace.html)
/// for definitions of these classes.
inline bool is_whitespace(char c) {
    // This is Pattern_White_Space.
    //
    // Note that this set is stable (ie, it doesn't change with different
    // Unicode versions), so it's ok to just hard-code the values.

    switch (c) {
        // Usual ASCII suspects
        case 0x0009: // \t
        case 0x000A: // \n
        case 0x000B: // Vertical tab
        case 0x000C: // form feed
        case 0x000D: // \r
        case 0x0020: // space
        
        // NEXT LINE from Latin-1
        case 0x0085: 
        
        // Bidi markers
        case 0x200E: // LEFT-TO-RIGHT MARK
        case 0x200F: // RIGHT-TO-LEFT MARK

        // Dedicated whitespace characters from Unicode
        case 0x2028: // LINE SEPARATOR
        case 0x2029: // PARAGRAPH SEPARATOR
            return true;

        default:
            return false;
    }
}
Token Cursor::advance_token() {
    // Get the first character, or return EOF if none.
    auto optional_first_char = bump();
    if (!optional_first_char || *optional_first_char == '\0') {
        return Token(TokenKind::Eof, 0);
    }

    char c = *optional_first_char;
    TokenKind token_kind;

    switch (c) {
        // Slash, comment, or block comment.
        case '/':
            switch (first()) {
                case '/': token_kind = line_comment(); break;
                case '*': token_kind = block_comment(); break;
                default: token_kind = TokenKind::Slash; break;
            }
            break;

        // **Whitespace sequence**
        default:
            if (is_whitespace(c)) {
                return whitespace();
            }

        // **Raw identifier, raw string literal, or identifier**
        case 'r':
            switch (first()) {
                case '#':
                    if (is_id_start(second())) {
                        token_kind = raw_ident();
                    } else {
                        auto res = raw_double_quoted_string(1);
                        uint8_t hash_count = std::holds_alternative<uint8_t>(res) ? std::get<uint8_t>(res) : 0;

                        uint32_t suffix_start = pos_within_token();
                        eat_literal_suffix();

                        return Token(TokenKind::Literal, LiteralKind::RawStrLiteral(hash_count), suffix_start);
                    }
                    break;
                case '"': {
                    auto res = raw_double_quoted_string(1);
                    uint8_t hash_count = std::holds_alternative<uint8_t>(res) ? std::get<uint8_t>(res) : 0;

                    uint32_t suffix_start = pos_within_token();
                    eat_literal_suffix();

                    return Token(TokenKind::Literal, LiteralKind::RawStrLiteral(hash_count), suffix_start);
                }
                default:
                    token_kind = ident_or_unknown_prefix();
            }
            break;

        // **Byte literal, byte string literal, raw byte string literal, or identifier**
        case 'b':
            return c_or_byte_string(
                [](bool terminated) { return LiteralKind::ByteStrLiteral(terminated); },
                [](std::optional<uint8_t> n_hashes) { return LiteralKind::RawByteStrLiteral(n_hashes.value_or(0)); },
                [](bool terminated) { return LiteralKind::ByteLiteral(terminated); }
            );

        // **C-string literal, raw C-string literal, or identifier**
        case 'c':
            return c_or_byte_string(
                [](bool terminated) { return LiteralKind::CStrLiteral(terminated); },
                [](std::optional<uint8_t> n_hashes) { return LiteralKind::RawCStrLiteral(n_hashes.value_or(0)); },
                std::nullopt
            );

        // **Guarded string literal prefix: `#"` or `##`**
        case '#':
            if (first() == '"' || first() == '#') {
                bump();
                token_kind = TokenKind::GuardedStrPrefix;
            }
            break;

        // **Lifetime or character literal**
        case '\'':
            return lifetime_or_char();

        // **String literal**
        case '"': {
            bool terminated = double_quoted_string();
            uint32_t suffix_start = pos_within_token();
            if (terminated) {
                eat_literal_suffix();
            }
            return Token(TokenKind::Literal, LiteralKind::StrLiteral(terminated), suffix_start);
        }

        // **Identifier**
        default:
            if (is_id_start(c)) {
                token_kind = ident_or_unknown_prefix();
            } 
            // **Numeric literal**
            else if (std::isdigit(c)) {
                auto literal_kind = number(c);
                uint32_t suffix_start = pos_within_token();
                eat_literal_suffix();
                return Token(TokenKind::Literal, literal_kind, suffix_start);
            } 
            // **Identifier starting with an emoji (error recovery case)**
            else if (!isascii(c) && is_emoji_char(c)) {
                token_kind = invalid_ident();
            } 
            // **One-symbol tokens**
            else {
                switch (c) {
                    case ';': token_kind = TokenKind::Semi; break;
                    case ',': token_kind = TokenKind::Comma; break;
                    case '.': token_kind = TokenKind::Dot; break;
                    case '(': token_kind = TokenKind::OpenParen; break;
                    case ')': token_kind = TokenKind::CloseParen; break;
                    case '{': token_kind = TokenKind::OpenBrace; break;
                    case '}': token_kind = TokenKind::CloseBrace; break;
                    case '[': token_kind = TokenKind::OpenBracket; break;
                    case ']': token_kind = TokenKind::CloseBracket; break;
                    case '@': token_kind = TokenKind::At; break;
                    case '#': token_kind = TokenKind::Pound; break;
                    case '~': token_kind = TokenKind::Tilde; break;
                    case '?': token_kind = TokenKind::Question; break;
                    case ':': token_kind = TokenKind::Colon; break;
                    case '$': token_kind = TokenKind::Dollar; break;
                    case '=': token_kind = TokenKind::Eq; break;
                    case '!': token_kind = TokenKind::Bang; break;
                    case '<': token_kind = TokenKind::Lt; break;
                    case '>': token_kind = TokenKind::Gt; break;
                    case '-': token_kind = TokenKind::Minus; break;
                    case '&': token_kind = TokenKind::And; break;
                    case '|': token_kind = TokenKind::Or; break;
                    case '+': token_kind = TokenKind::Plus; break;
                    case '*': token_kind = TokenKind::Star; break;
                    case '/': token_kind = TokenKind::Slash; break;
                    case '^': token_kind = TokenKind::Caret; break;
                    case '%': token_kind = TokenKind::Percent; break;
                    default: token_kind = TokenKind::Unknown; break;
                }
            }
    }

    uint32_t token_length = pos_within_token();
    reset_pos_within_token();
    return Token(token_kind, token_length);
}

TokenKind Cursor::whitespace() {
    cursor.eat_while([](char c) { return std::isspace(c); });
    return TokenKind::Whitespace;
}

TokenKind Cursor::raw_ident() {
    assert(prev() == 'r' && first() == '#' && is_id_start(second()));
    // Eat "#" symbol.
    bump();
    // Eat the identifier part of RawIdent.
    eat_identifier();
    return TokenKind::RawIdent;
}

/// Parses an identifier or an unknown prefix.
TokenKind Cursor::ident_or_unknown_prefix() {
    eat_while([](char c) { return std::isalnum(c) || c == '_'; });
    return TokenKind::Ident;
}

/// Parses an invalid identifier, consuming the invalid sequence.
TokenKind Cursor::invalid_ident() {
    // Consume the remaining identifier characters, including emoji sequences.
    eat_while([](char c) {
        constexpr char ZERO_WIDTH_JOINER = '\u200d';
        return is_id_continue(c) || (!isascii(c) && is_emoji_char(c)) || c == ZERO_WIDTH_JOINER;
    });

    return TokenKind::InvalidIdent;
}

/// Parses a `c` or `b` prefixed string, raw string, or identifier.
Token Cursor::c_or_byte_string(
    std::function<LiteralKind(bool)> mk_kind,
    std::function<LiteralKind(std::optional<uint8_t>)> mk_kind_raw,
    std::optional<std::function<LiteralKind(bool)>> single_quoted
) {
    switch (std::make_tuple(first(), second(), single_quoted.has_value())) {
        case std::make_tuple('\'', _, true): {  // Single-quoted literal
            bump();
            bool terminated = single_quoted_string();
            uint32_t suffix_start = pos_within_token();
            if (terminated) eat_literal_suffix();

            return Token(TokenKind::Literal, single_quoted.value()(terminated), suffix_start);
        }
        case std::make_tuple('"', _, _): {  // Double-quoted literal
            bump();
            bool terminated = double_quoted_string();
            uint32_t suffix_start = pos_within_token();
            if (terminated) eat_literal_suffix();

            return Token(TokenKind::Literal, mk_kind(terminated), suffix_start);
        }
        case std::make_tuple('r', '"', _) :
        case std::make_tuple('r', '#', _) : {  // Raw string literal
            bump();
            auto res = raw_double_quoted_string(2);
            uint8_t hash_count = std::holds_alternative<uint8_t>(res) ? std::get<uint8_t>(res) : 0;
            uint32_t suffix_start = pos_within_token();
            if (res.index() == 0) eat_literal_suffix();  // Check for successful parsing

            return Token(TokenKind::Literal, mk_kind_raw(hash_count), suffix_start);
        }
        default:
            return ident_or_unknown_prefix();
    }
}

LiteralKind Cursor::number(char first_digit) {
    assert(prev() >= '0' && prev() <= '9');  // Ensure previous character is a digit.
    Base base = Base::Decimal;

    if (first_digit == '0') {
        // Attempt to parse encoding base.
        switch (first()) {
            case 'b': {
                base = Base::Binary;
                bump();
                if (!eat_decimal_digits()) {
                    return LiteralKind::IntLiteral(base, true);
                }
                break;
            }
            case 'o': {
                base = Base::Octal;
                bump();
                if (!eat_decimal_digits()) {
                    return LiteralKind::IntLiteral(base, true);
                }
                break;
            }
            case 'x': {
                base = Base::Hexadecimal;
                bump();
                if (!eat_hexadecimal_digits()) {
                    return LiteralKind::IntLiteral(base, true);
                }
                break;
            }
            case '0' ... '9': case '_':
                eat_decimal_digits();
                break;
            case '.': case 'e': case 'E':
                break;
            default:
                return LiteralKind::IntLiteral(base, false);
        }
    } else {
        // No base prefix, parse number in the usual way.
        eat_decimal_digits();
    }

    switch (first()) {
        case '.':
            if (second() != '.' && !is_id_start(second())) {
                bump();
                bool empty_exponent = false;
                if (isdigit(first())) {
                    eat_decimal_digits();
                    if (first() == 'e' || first() == 'E') {
                        bump();
                        empty_exponent = !eat_float_exponent();
                    }
                }
                return LiteralKind::FloatLiteral(base, empty_exponent);
            }
            break;
        case 'e': case 'E':
            bump();
            return LiteralKind::FloatLiteral(base, !eat_float_exponent());
        default:
            return LiteralKind::IntLiteral(base, false);
    }
    return LiteralKind::IntLiteral(base, false);
}

TokenKind Cursor::lifetime_or_char() {
    assert(prev() == '\'');

    bool can_be_a_lifetime = true;

    if (second() == '\'') {
        // It's surely not a lifetime.
        can_be_a_lifetime = false;
    } else {
        // If the first symbol is valid for an identifier, it can be a lifetime.
        // Also check if it's a number for better error reporting (so '0 will
        // be reported as an invalid lifetime and not as an unterminated char literal).
        can_be_a_lifetime = is_id_start(first()) || isdigit(first());
    }

    if (!can_be_a_lifetime) {
        bool terminated = single_quoted_string();
        uint32_t suffix_start = pos_within_token();
        if (terminated) {
            eat_literal_suffix();
        }
        return Token(TokenKind::Literal, LiteralKind::CharLiteral(terminated), suffix_start);
    }

    if (first() == 'r' && second() == '#' && is_id_start(third())) {
        // Eat "r" and `#`, and identifier start characters.
        bump();
        bump();
        bump();
        eat_while(is_id_continue);
        return TokenKind::RawLifetime;
    }

    // Either a lifetime or a character literal with length greater than 1.
    bool starts_with_number = isdigit(first());

    // Skip the literal contents.
    bump();
    eat_while(is_id_continue);

    switch (first()) {
        case '\'':
            bump();
            return Token(TokenKind::Literal, LiteralKind::CharLiteral(true), pos_within_token());
        case '#':
            if (!starts_with_number) {
                return TokenKind::UnknownPrefixLifetime;
            }
            break;
    }
    return TokenKind::Lifetime;
}

bool Cursor::double_quoted_string() {
    assert(prev() == '"');  // Rust `debug_assert!()` -> C++ `assert()`

    while (auto c = bump()) {  // Simulating `while let Some(c) = self.bump()`
        if (*c == '"') {
            return true;
        }
        if (*c == '\\' && (first() == '\\' || first() == '"')) {
            bump();  // Skip escaped character
        }
    }
    // End of file reached.
    return false;
}

std::optional<GuardedStr> Cursor::guarded_double_quoted_string() {
    assert(prev() != '#');

    uint32_t n_start_hashes = 0;
    while (first() == '#') {
        n_start_hashes++;
        bump();
    }

    if (first() != '"') {
        return std::nullopt;
    }
    bump();
    assert(prev() == '"');

    // Parse string as a normal string literal
    bool terminated = double_quoted_string();
    if (!terminated) {
        uint32_t token_len = pos_within_token();
        reset_pos_within_token();
        return GuardedStr{n_start_hashes, false, token_len};
    }

    // Consume closing `#` symbols (but don't consume extra trailing `#` characters)
    uint32_t n_end_hashes = 0;
    while (first() == '#' && n_end_hashes < n_start_hashes) {
        n_end_hashes++;
        bump();
    }

    eat_literal_suffix();

    uint32_t token_len = pos_within_token();
    reset_pos_within_token();

    return GuardedStr{n_start_hashes, true, token_len};
}

std::variant<uint8_t, RawStrError> Cursor::raw_double_quoted_string(uint32_t prefix_len) {
    auto n_hashes = raw_string_unvalidated(prefix_len);
    if (std::holds_alternative<RawStrError>(n_hashes)) {
        return std::get<RawStrError>(n_hashes);
    }

    // Only up to 255 `#`s are allowed in raw strings
    uint32_t hash_count = std::get<uint32_t>(n_hashes);
    if (hash_count > 255) {
        return RawStrError::TooManyDelimiters{hash_count};
    }
    return static_cast<uint8_t>(hash_count);
}

std::variant<uint32_t, RawStrError> Cursor::raw_string_unvalidated(uint32_t prefix_len) {
    assert(prev() == 'r');
    uint32_t start_pos = pos_within_token();
    std::optional<uint32_t> possible_terminator_offset = std::nullopt;
    uint32_t max_hashes = 0;

    // Count opening `#` symbols
    uint32_t n_start_hashes = 0;
    while (first() == '#') {
        n_start_hashes++;
        bump();
    }

    // Check that string starts correctly
    if (bump() != '"') {
        return RawStrError::InvalidStarter{first()};
    }

    // Skip contents, checking for raw string termination
    while (true) {
        eat_until('"');

        if (is_eof()) {
            return RawStrError::NoTerminator{n_start_hashes, max_hashes, possible_terminator_offset};
        }

        bump();  // Eat closing double quote

        uint32_t n_end_hashes = 0;
        while (first() == '#' && n_end_hashes < n_start_hashes) {
            n_end_hashes++;
            bump();
        }

        if (n_end_hashes == n_start_hashes) {
            return n_start_hashes;
        }

        if (n_end_hashes > max_hashes) {
            possible_terminator_offset = pos_within_token() - start_pos - n_end_hashes + prefix_len;
            max_hashes = n_end_hashes;
        }
    }
}

bool Cursor::eat_decimal_digits() {
    bool has_digits = false;
    while (true) {
        switch (first()) {
            case '_':
                bump();
                break;
            case '0' ... '9':
                has_digits = true;
                bump();
                break;
            default:
                return has_digits;
        }
    }
}

bool Cursor::eat_hexadecimal_digits() {
    bool has_digits = false;
    while (true) {
        switch (first()) {
            case '_':
                bump();
                break;
            case '0' ... '9': case 'a' ... 'f': case 'A' ... 'F':
                has_digits = true;
                bump();
                break;
            default:
                return has_digits;
        }
    }
}

bool Cursor::eat_float_exponent() {
    assert(prev() == 'e' || prev() == 'E');

    if (first() == '-' || first() == '+') {
        bump();
    }
    return eat_decimal_digits();
}

void Cursor::eat_literal_suffix() {
    eat_identifier();
}

void Cursor::eat_identifier() {
    if (!is_id_start(first())) {
        return;
    }
    bump();
    eat_while(is_id_continue);
}





TokenKind Cursor::comment() {
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



TokenKind Cursor::lex_number() {
    cursor.eat_while([](char c) { return std::isdigit(c); });
    return TokenKind::Literal;
}

TokenKind Cursor::lex_punctuation() {
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
TokenKind Cursor::lifetime_or_char() {
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
bool Cursor::single_quoted_string() {
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
bool Cursor::double_quoted_string() {
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
std::optional<GuardedStr> Cursor::guarded_double_quoted_string() {
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

// std::optional<uint8_t> Cursor::raw_double_quoted_string(uint32_t prefix_len) {
//     // Wrap the actual function to handle the error with too many hashes.
//     // This way, it eats the whole raw string.
//     auto n_hashes = raw_string_unvalidated(prefix_len);

//     // Only up to 255 `#`s are allowed in raw strings
//     if (n_hashes > 255) {
//         return std::nullopt; // Indicate error: Too many delimiters
//     }

//     return static_cast<uint8_t>(n_hashes); // Return the valid hash count
// }

// Parses a raw double-quoted string literal
std::variant<uint8_t, RawStrErrorDetails> Cursor::raw_double_quoted_string(Cursor& cursor, uint32_t prefix_len) {
    // Wrap the actual function to handle the error with too many hashes.
    // This way, it eats the whole raw string.
    std::optional<uint32_t> n_hashes = raw_string_unvalidated(cursor, prefix_len);
    
    if (!n_hashes.has_value()) {
        return RawStrErrorDetails(RawStrError::NoTerminator, '\0', prefix_len, 0);
    }

    // Only up to 255 `#`s are allowed in raw strings
    if (n_hashes.value() > 255) {
        return RawStrErrorDetails(RawStrError::TooManyDelimiters, '\0', 255, n_hashes.value());
    }

    return static_cast<uint8_t>(n_hashes.value());
}

// Internal function to validate and parse raw strings
uint32_t Cursor::raw_string_unvalidated(uint32_t prefix_len) {
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
bool Cursor::eat_decimal_digits() {
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
bool Cursor::eat_hexadecimal_digits() {
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
bool Cursor::eat_float_exponent() {
    assert(cursor.prev() == 'e' || cursor.prev() == 'E');

    if (cursor.first() == '-' || cursor.first() == '+') {
        cursor.bump();
    }
    return eat_decimal_digits();
}

// Eats the suffix of a literal (e.g., `u8`)
void Cursor::eat_literal_suffix() {
    eat_identifier();
}

// Consumes an identifier
void Cursor::eat_identifier() {
    if (!is_id_start(cursor.first())) {
        return;
    }
    cursor.bump();
    cursor.eat_while([this](char c) { return is_id_continue(c); });
}

// Checks if a character is a valid identifier start
bool Cursor::is_id_start(char c) {
    return std::isalpha(c) || c == '_';
}

// Checks if a character can continue an identifier
bool Cursor::is_id_continue(char c) {
    return std::isalnum(c) || c == '_';
}
