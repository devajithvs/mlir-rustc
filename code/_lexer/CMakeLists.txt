cmake_minimum_required(VERSION 3.10)
project(RustLexer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define the library
add_library(rust_lexer STATIC 
    src/Lexer.cpp 
    src/Unescape.cpp 
)

target_include_directories(rust_lexer PUBLIC include)

# Unit tests
include(GoogleTest)
enable_testing()
add_executable(lexer_tests tests/LexerTests.cpp)
target_link_libraries(lexer_tests PRIVATE rust_lexer gtest gtest_main)


add_test(NAME LexerTest COMMAND lexer_tests)
