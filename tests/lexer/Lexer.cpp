#include "Lexer/Lexer.h"

#include "gtest/gtest.h"

using namespace rust_compiler::lexer;

TEST(LexerTest, CheckKeyWord) {

  std::string text = "for async\n";

  TokenStream ts = lex(text, "lib.rs");

  size_t expectedLendth = 2;

  EXPECT_EQ(ts.getLength(), expectedLendth);
};

TEST(LexerTest, CheckUse) {
  std::string text = "use "
                     "aws_sdk_ec2::{error::DescribeInstanceTypesError,model::{"
                     "InstanceType, InstanceTypeInfo}, types::SdkError,};\n";

  TokenStream ts = lex(text, "lib.rs");

  size_t expectedLendth = 22;

  EXPECT_EQ(ts.getLength(), expectedLendth);
};


TEST(LexerTest, CheckInteger) {
  std::string text = "128";

  TokenStream ts = lex(text, "lib.rs");

  size_t expectedLendth = 1;

  EXPECT_EQ(ts.getLength(), expectedLendth);

  EXPECT_EQ(ts.getAsView().front().getKind(), TokenKind::DecIntegerLiteral);

};


TEST(LexerTest, CheckDecInteger) {
  std::string text = "1";

  TokenStream ts = lex(text, "lib.rs");

  size_t expectedLendth = 1;

  EXPECT_EQ(ts.getLength(), expectedLendth);

  EXPECT_EQ(ts.getAsView().front().getKind(), TokenKind::DecIntegerLiteral);

};


TEST(LexerTest, CheckDollarCrate) {
  std::string text = "$crate";

  TokenStream ts = lex(text, "lib.rs");

  size_t expectedLendth = 1;

  EXPECT_EQ(ts.getLength(), expectedLendth);

  //EXPECT_EQ(ts.getAsView().front().getKind(), TokenKind::DecIntegerLiteral);

};
