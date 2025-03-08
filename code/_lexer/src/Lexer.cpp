#include "Lexer.h"

/// `rustc` allows files to have a shebang, e.g. "#!/usr/bin/rustrun",
/// but shebang isn't a part of Rust syntax.
/// This function removes the shebang if present.
std::optional<size_t> strip_shebang(const std::string& input) {
    if (input.rfind("#!", 0) == 0) {  // Ensure it starts with `#!`
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
    if (input.empty()) {
        return RawStrErrorDetails(RawStrError::InvalidStarter, '\0');
    }

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
