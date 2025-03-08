#ifndef LEXER_H
#define LEXER_H

#include "Token.h"
#include "Cursor.h"
#include <vector>
#include <variant>

std::optional<size_t> strip_shebang(const std::string& input);
std::variant<std::monostate, RawStrErrorDetails> validate_raw_str(const std::string& input, uint32_t prefix_len);

class Lexer {
public:
    std::vector<Token> tokens;

    TokenKind advance_token();  // Ensure function is declared
    TokenKind ident_or_unknown_prefix();  // Ensure function is declared
    TokenKind line_comment();
    TokenKind lex_identifier();
    TokenKind lex_number();
    TokenKind lex_literal();
    TokenKind lex_punctuation();
    TokenKind lex_whitespace();

    // Parses a character literal or a lifetime
    TokenKind lifetime_or_char();

    // Parses a single-quoted character literal
    bool single_quoted_string();

    // Parses a double-quoted string literal
    bool double_quoted_string();

    // Parses a guarded string literal with `#"` or `##"` (reserved syntax for future Rust editions)
    std::optional<GuardedStr> guarded_double_quoted_string();

    // Parses a raw double-quoted string literal
    std::optional<uint8_t> raw_double_quoted_string(uint32_t prefix_len);

    // Internal function to validate and parse raw strings
    uint32_t raw_string_unvalidated(uint32_t prefix_len);

    // Helper function to eat decimal digits
    bool eat_decimal_digits();

    // Helper function to eat hexadecimal digits
    bool eat_hexadecimal_digits();

    // Parses a float exponent
    bool eat_float_exponent();

    // Eats the suffix of a literal (e.g., `u8`)
    void eat_literal_suffix();

    // Consumes an identifier
    void eat_identifier();

    // Checks if a character is a valid identifier start
    bool is_id_start(char c);

    // Checks if a character can continue an identifier
    bool is_id_continue(char c);

public:
    Cursor cursor;
    explicit Lexer(std::string_view input) : cursor(input) {}

    std::vector<Token> tokenize();
};

#endif // LEXER_H
