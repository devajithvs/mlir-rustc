#include "Session/Session.h"
#include "TyCtx/TyCtx.h"
#include "gtest/gtest.h"

int main(int argc, char **argv) {
  // Ensure session is initialized before any test runs
  rust_compiler::session::session =
      new rust_compiler::session::Session(0, nullptr);

  ::testing::InitGoogleTest(&argc, argv);
  int result = RUN_ALL_TESTS();

  // Cleanup after tests
  delete rust_compiler::session::session;
  rust_compiler::session::session = nullptr;

  return result;
}
