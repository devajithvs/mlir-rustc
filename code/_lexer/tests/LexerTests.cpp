#include "Lexer.h"
#include "Cursor.h"
#include "Unescape.h"
#include <gtest/gtest.h>
#include <vector>

// Testing invalid character escapes
TEST(UnescapeTest, InvalidEscapes) {
    EXPECT_EQ(unescape_char(""), std::nullopt); // ZeroChars
    EXPECT_EQ(unescape_char("\\"), std::nullopt); // LoneSlash
    EXPECT_EQ(unescape_char("\n"), std::nullopt); // EscapeOnlyChar
    EXPECT_EQ(unescape_char("\t"), std::nullopt); // EscapeOnlyChar
    EXPECT_EQ(unescape_char("'"), std::nullopt); // EscapeOnlyChar
    EXPECT_EQ(unescape_char("\r"), std::nullopt); // BareCarriageReturn
}

// Testing valid character escapes
TEST(UnescapeTest, ValidEscapes) {
    EXPECT_EQ(unescape_char("a"), 'a');
    EXPECT_EQ(unescape_char("\n"), '\n');
    EXPECT_EQ(unescape_char("\r"), '\r');
    EXPECT_EQ(unescape_char("\t"), '\t');
    EXPECT_EQ(unescape_char("\\\""), '"');
    EXPECT_EQ(unescape_char("\\\\"), '\\');
    EXPECT_EQ(unescape_char("\\\'"), '\'');
    EXPECT_EQ(unescape_char("\\0"), '\0');
}

// Test raw string lexing
TEST(LexerTest, RawStringLiteral) {
    std::string input = "r###\"hello\"###";
    Cursor cursor(input);
    cursor.bump();
    auto token = cursor.raw_double_quoted_string(0);
    ASSERT_TRUE(token.has_value());
    EXPECT_EQ(token.value(), 3);
}

// Check lexing of identifiers
TEST(LexerTest, Identifiers) {
    std::string input = "fn main() { println!(\"hello\"); }";
    Lexer lexer(input);
    auto tokens = lexer.tokenize();

    ASSERT_GT(tokens.size(), 0);
    EXPECT_EQ(tokens[0].kind, TokenKind::Ident);
}

// Test invalid lexing cases
TEST(LexerTest, InvalidLexing) {
    std::string input = "#!";
    Lexer lexer(input);
    auto tokens = lexer.tokenize();

    EXPECT_FALSE(tokens.empty());
    EXPECT_EQ(tokens[0].kind, TokenKind::Unknown);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
