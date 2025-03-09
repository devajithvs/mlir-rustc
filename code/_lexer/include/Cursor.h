#ifndef CURSOR_H
#define CURSOR_H

#include <string>
#include <functional>
#include <optional>
#include <variant>
#include "Token.h"

class Cursor {
public:
    std::string_view input;
    size_t position = 0;

    TokenKind advance_token();  // Ensure function is declared
    TokenKind ident_or_unknown_prefix();  // Ensure function is declared
    TokenKind line_comment();
    TokenKind lex_identifier();
    TokenKind lex_number();
    TokenKind lex_literal();
    TokenKind lex_punctuation();
    TokenKind lex_whitespace();
    TokenKind raw_ident()

    // Parses a character literal or a lifetime
    TokenKind lifetime_or_char();

    // Parses a single-quoted character literal
    bool single_quoted_string();

    // Parses a double-quoted string literal
    bool double_quoted_string();

    // Parses a guarded string literal with `#"` or `##"` (reserved syntax for future Rust editions)
    std::optional<GuardedStr> guarded_double_quoted_string();

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


    explicit Cursor(std::string_view input) : input(input) {}

    inline char first() const {
        return position < input.size() ? input[position] : '\0';
    }

    inline char second() const {
        return (position + 1) < input.size() ? input[position + 1] : '\0';
    }

    inline char third() const {
        return (position + 2) < input.size() ? input[position + 2] : '\0';
    }

    inline bool is_eof() const {
        return position >= input.size();
    }

    inline std::optional<char> bump() {
        if (position < input.size()) {
            return input[position++];
        }
        return std::nullopt;  // Return an empty optional instead of void
    }    

    void eat_while(std::function<bool(char)> predicate) {
        while (!is_eof() && predicate(first())) {
            bump();
        }
    }

    void eat_until(char target) {
        while (!is_eof() && first() != target) {
            bump();
        }
    }

    // Store the previous character
    char prev() const {
        return (position == 0) ? '\0' : input[position - 1];
    }

    // Return the position of the current token
    uint32_t pos_within_token() const {
        return position;
    }

    // Reset position within a token
    void reset_pos_within_token() {
        position = 0;
    }

    std::variant<uint8_t, RawStrErrorDetails> raw_double_quoted_string(uint32_t);


};

#endif // CURSOR_H
