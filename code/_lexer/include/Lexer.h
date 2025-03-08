#ifndef LEXER_H
#define LEXER_H

#include "Token.h"
#include "Cursor.h"
#include <vector>
#include <variant>

class Lexer {
private:
    Cursor cursor;
    std::vector<Token> tokens;

    TokenKind advance_token();  // Ensure function is declared
    TokenKind ident_or_unknown_prefix();  // Ensure function is declared
    TokenKind line_comment();
    TokenKind lex_identifier();
    TokenKind lex_number();
    TokenKind lex_literal();
    TokenKind lex_punctuation();
    TokenKind lex_whitespace();

public:
    explicit Lexer(std::string_view input) : cursor(input) {}

    std::vector<Token> tokenize();
};

#endif // LEXER_H
