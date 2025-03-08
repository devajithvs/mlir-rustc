#include "Lexer.h"
#include "Cursor.h"
#include "Unescape.h"
#include <gtest/gtest.h>
#include <vector>
#include <optional>

// Helper function to check raw string lexing
void check_raw_str(const std::string& s, std::optional<uint8_t> expected, std::optional<RawStrError> error = std::nullopt) {
    std::string input = "r" + s; // Simulating Rust’s format!("r{}", s)
    Lexer lexer(input);
    Cursor cursor = lexer.cursor;
    cursor.bump();
    
    auto res = lexer.raw_double_quoted_string(0);
    
    if (error.has_value()) {
        EXPECT_FALSE(res.has_value());  // Ensure it fails
    } else {
        ASSERT_TRUE(res.has_value());
        EXPECT_EQ(res.value(), expected.value());
    }
}

// Test: Valid raw string parsing
TEST(LexerTest, NakedRawStr) {
    check_raw_str(R"("abc")", 0);
}

TEST(LexerTest, RawNoStart) {
    check_raw_str(R"(""abc"#")", 0);
}

TEST(LexerTest, TooManyTerminators) {
    check_raw_str(R"("#"abc"##")", 1);
}

TEST(LexerTest, UnterminatedRawStr) {
    check_raw_str(R"("#"abc"#)", std::nullopt, RawStrError::NoTerminator);
    check_raw_str(R"("##"abc"#")", std::nullopt, RawStrError::NoTerminator);
}

TEST(LexerTest, InvalidRawStrStart) {
    check_raw_str(R"("#~"abc"#")", std::nullopt, RawStrError::InvalidStarter);
}

TEST(LexerTest, TooManyHashes) {
    std::string max_hashes(255, '#'); // 255 `#` characters
    std::string too_many_hashes(256, '#'); // 256 `#` characters

    std::string s1 = max_hashes + R"("abc")" + max_hashes;
    std::string s2 = too_many_hashes + R"("abc")" + too_many_hashes;

    check_raw_str(s1, 255); // Valid case
    check_raw_str(s2, std::nullopt, RawStrError::TooManyDelimiters);
}

// Test: Shebang stripping
TEST(LexerTest, ValidShebang) {
    EXPECT_EQ(strip_shebang("#!/bin/bash"), std::optional<size_t>(11));
    EXPECT_EQ(strip_shebang("#![attribute]"), std::nullopt);
    EXPECT_EQ(strip_shebang("#!    /bin/bash"), std::optional<size_t>(15));
    EXPECT_EQ(strip_shebang("#! // comment\n/bin/bash"), std::optional<size_t>(10));
    EXPECT_EQ(strip_shebang("\n#!/bin/bash"), std::nullopt); // Not first line
}

// Test: Lexing behavior
void check_lexing(const std::string& src, const std::vector<TokenKind>& expected_tokens) {
    Lexer lexer(src);
    auto tokens = lexer.tokenize();

    ASSERT_EQ(tokens.size(), expected_tokens.size());
    for (size_t i = 0; i < tokens.size(); i++) {
        EXPECT_EQ(tokens[i].kind, expected_tokens[i]);
    }
}

// Smoke test for lexing
TEST(LexerTest, SmokeTest) {
    check_lexing(
        "/* my source file */ fn main() { println!(\"zebra\"); }",
        {TokenKind::BlockComment, TokenKind::Whitespace, TokenKind::Ident, TokenKind::Whitespace, 
         TokenKind::Ident, TokenKind::OpenParen, TokenKind::CloseParen, TokenKind::Whitespace, 
         TokenKind::OpenBrace, TokenKind::Whitespace, TokenKind::Ident, TokenKind::Bang, 
         TokenKind::OpenParen, TokenKind::Literal, TokenKind::CloseParen, TokenKind::Semi, 
         TokenKind::Whitespace, TokenKind::CloseBrace, TokenKind::Whitespace});
}

// Test: Different types of comments
TEST(LexerTest, CommentFlavors) {
    check_lexing(
        R"(
// line
//// line as well
/// outer doc line
//! inner doc line
/* block */
/**/
/*** also block */
/** outer doc block */
/*! inner doc block */
)",
        {TokenKind::Whitespace, TokenKind::LineComment, TokenKind::Whitespace, TokenKind::LineComment,
         TokenKind::Whitespace, TokenKind::LineComment, TokenKind::Whitespace, TokenKind::LineComment,
         TokenKind::Whitespace, TokenKind::BlockComment, TokenKind::Whitespace, TokenKind::BlockComment,
         TokenKind::Whitespace, TokenKind::BlockComment, TokenKind::Whitespace, TokenKind::BlockComment,
         TokenKind::Whitespace, TokenKind::BlockComment, TokenKind::Whitespace});
}

// Test: Nested block comments
TEST(LexerTest, NestedBlockComments) {
    check_lexing("/* /* */ */'a'", {TokenKind::BlockComment, TokenKind::Literal});
}

// Test: Character literals
TEST(LexerTest, Characters) {
    check_lexing("'a' ' ' '\\n'", {TokenKind::Literal, TokenKind::Whitespace, TokenKind::Literal,
                                   TokenKind::Whitespace, TokenKind::Literal});
}

// Test: Lifetimes
TEST(LexerTest, Lifetime) {
    check_lexing("'abc", {TokenKind::Lifetime});
}

// Test: Raw strings
TEST(LexerTest, RawString) {
    check_lexing(R"(r###""#a\b\x00c""###)", {TokenKind::Literal});
}

// Test: Literal suffixes
TEST(LexerTest, LiteralSuffixes) {
    check_lexing(
        R"(
'a'
b'a'
"a"
b"a"
1234
0b101
0xABC
1.0
1.0e10
2us
r###"raw"###suffix
br###"raw"###suffix
)",
        {TokenKind::Whitespace, TokenKind::Literal, TokenKind::Whitespace, TokenKind::Literal,
         TokenKind::Whitespace, TokenKind::Literal, TokenKind::Whitespace, TokenKind::Literal,
         TokenKind::Whitespace, TokenKind::Literal, TokenKind::Whitespace, TokenKind::Literal,
         TokenKind::Whitespace, TokenKind::Literal, TokenKind::Whitespace, TokenKind::Literal,
         TokenKind::Whitespace, TokenKind::Literal, TokenKind::Whitespace, TokenKind::Literal,
         TokenKind::Whitespace, TokenKind::Literal, TokenKind::Whitespace, TokenKind::Literal});
}

// **Main function to run all tests**
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
