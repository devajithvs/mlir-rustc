#include "Lexer/Lexer.h"
#include "Parser/Parser.h"
#include "gtest/gtest.h"

#include <llvm/Support/raw_ostream.h>

using namespace rust_compiler::lexer;
using namespace rust_compiler::parser;
using namespace rust_compiler::ast;
using namespace rust_compiler::adt;

TEST(SlicePatternTest, CheckSlicePattern3) {
  std::string text = "[]";

  TokenStream ts = lex(text, "lib.rs");

  //ts.print(10);

  Parser parser = {ts};

  Result<std::shared_ptr<rust_compiler::ast::patterns::PatternNoTopAlt>,
         std::string>
      result = parser.parseSlicePattern();

  EXPECT_TRUE(result.isOk());
}

TEST(SlicePatternTest, CheckSlicePattern2) {
  std::string text = "[.., last]";

  TokenStream ts = lex(text, "lib.rs");

  //ts.print(10);

  Parser parser = {ts};

  Result<std::shared_ptr<rust_compiler::ast::patterns::PatternNoTopAlt>,
         std::string>
      result = parser.parseSlicePattern();

  if (result.isErr())
    llvm::errs() << result.getError() << "\n";

  EXPECT_TRUE(result.isOk());
}

TEST(SlicePatternTest, CheckSlicePattern1) {
  std::string text = "[first, ..]";

  TokenStream ts = lex(text, "lib.rs");

  //ts.print(10);

  Parser parser = {ts};

  Result<std::shared_ptr<rust_compiler::ast::patterns::PatternNoTopAlt>,
         std::string>
      result = parser.parseSlicePattern();

  EXPECT_TRUE(result.isOk());
}
