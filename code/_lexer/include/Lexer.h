#ifndef LEXER_H
#define LEXER_H

#include "Token.h"
#include "Cursor.h"
#include <vector>
#include <variant>

std::optional<size_t> strip_shebang(const std::string& input);
std::variant<std::monostate, RawStrErrorDetails> validate_raw_str(const std::string& input, uint32_t prefix_len);
std::vector<Token> tokenize(const std::string& input);

class Lexer {
public:
    std::vector<Token> tokens;
public:
    Cursor cursor;
    explicit Lexer(std::string_view input) : cursor(input) {}

    std::vector<Token> tokenize();
};

#endif // LEXER_H
