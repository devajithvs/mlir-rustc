#ifndef UNESCAPE_H
#define UNESCAPE_H

#include <string>
#include <vector>
#include <optional>
#include <variant>
#include <cstdint>
#include <stdexcept>

// Enum for different escape errors
enum class EscapeError {
    ZeroChars,
    MoreThanOneChar,
    LoneSlash,
    InvalidEscape,
    BareCarriageReturn,
    BareCarriageReturnInRawString,
    EscapeOnlyChar,
    TooShortHexEscape,
    InvalidCharInHexEscape,
    OutOfRangeHexEscape,
    NoBraceInUnicodeEscape,
    InvalidCharInUnicodeEscape,
    EmptyUnicodeEscape,
    UnclosedUnicodeEscape,
    LeadingUnderscoreUnicodeEscape,
    OverlongUnicodeEscape,
    LoneSurrogateUnicodeEscape,
    OutOfRangeUnicodeEscape,
    UnicodeEscapeInByte,
    NonAsciiCharInByte,
    NulInCStr,
    UnskippedWhitespaceWarning,
    MultipleSkippedLinesWarning
};

// Enum to represent different parsing modes
enum class Mode {
    Char,
    Byte,
    Str,
    RawStr,
    ByteStr,
    RawByteStr,
    CStr,
    RawCStr
};

// Mixed unit type for ASCII and Unicode processing
class MixedUnit {
public:
    enum class Type { Char, HighByte };
    
    MixedUnit(char c) : type(Type::Char), value(c) {}
    MixedUnit(uint8_t b) : type(Type::HighByte), value(b) {}

    Type getType() const { return type; }
    char getChar() const { return std::get<char>(value); }
    uint8_t getByte() const { return std::get<uint8_t>(value); }

private:
    Type type;
    std::variant<char, uint8_t> value;
};

// Function declarations
std::variant<char, EscapeError> unescape_char(const std::string& src);
std::variant<uint8_t, EscapeError> unescape_byte(const std::string& src);
void unescape_unicode(const std::string& src, Mode mode, 
                      std::vector<std::pair<size_t, std::variant<char, EscapeError>>>& results);
void unescape_mixed(const std::string& src, Mode mode, 
                    std::vector<std::pair<size_t, std::variant<MixedUnit, EscapeError>>>& results);

#endif // UNESCAPE_H
