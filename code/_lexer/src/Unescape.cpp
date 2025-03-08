#include "Unescape.h"
#include <cctype>
#include <sstream>
#include <iostream>

std::variant<char, EscapeError> scan_escape(std::istringstream& stream, Mode mode) {
    char c;
    if (!(stream >> c)) return EscapeError::LoneSlash;

    switch (c) {
        case '"': return '"';
        case 'n': return '\n';
        case 'r': return '\r';
        case 't': return '\t';
        case '\\': return '\\';
        case '\'': return '\'';
        case '0': return '\0';
        case 'x': {
            char hi, lo;
            if (!(stream >> hi) || !(stream >> lo)) return EscapeError::TooShortHexEscape;
            if (!std::isxdigit(hi) || !std::isxdigit(lo)) return EscapeError::InvalidCharInHexEscape;
            uint8_t value = (std::stoi(std::string(1, hi), nullptr, 16) << 4) 
                          | std::stoi(std::string(1, lo), nullptr, 16);
            if (mode == Mode::Char && value > 127) return EscapeError::OutOfRangeHexEscape;
            return static_cast<char>(value);
        }
        case 'u': {
            if (stream.peek() != '{') return EscapeError::NoBraceInUnicodeEscape;
            stream.get(); // Consume '{'

            std::string hex;
            while (stream.peek() != '}' && stream.good()) {
                char next = stream.get();
                if (next == '_') continue; // Allow underscores in Unicode
                if (!std::isxdigit(next)) return EscapeError::InvalidCharInUnicodeEscape;
                hex += next;
            }
            
            if (stream.get() != '}') return EscapeError::UnclosedUnicodeEscape;
            if (hex.empty()) return EscapeError::EmptyUnicodeEscape;

            uint32_t value = std::stoul(hex, nullptr, 16);
            if (value > 0x10FFFF) return EscapeError::OutOfRangeUnicodeEscape;
            return static_cast<char>(value);
        }
        default:
            return EscapeError::InvalidEscape;
    }
}

std::variant<char, EscapeError> unescape_char(const std::string& src) {
    if (src.empty()) return EscapeError::ZeroChars;
    std::istringstream stream(src);
    
    char first;
    if (!(stream >> first)) return EscapeError::ZeroChars;
    
    std::variant<char, EscapeError> res;
    if (first == '\\') {
        res = scan_escape(stream, Mode::Char);
    } else {
        res = first;
    }

    if (!stream.eof()) return EscapeError::MoreThanOneChar;
    return res;
}

std::variant<uint8_t, EscapeError> unescape_byte(const std::string& src) {
    auto res = unescape_char(src);
    if (std::holds_alternative<char>(res)) {
        char c = std::get<char>(res);
        if (c >= 0 && static_cast<unsigned char>(c) <= 255) return static_cast<uint8_t>(c);
        return EscapeError::NonAsciiCharInByte;
    }
    return std::get<EscapeError>(res);
}

void unescape_unicode(const std::string& src, Mode mode,
                      std::vector<std::pair<size_t, std::variant<char, EscapeError>>>& results) {
    std::istringstream stream(src);
    size_t index = 0;
    
    while (stream.good()) {
        char c;
        if (!(stream >> c)) break;

        std::variant<char, EscapeError> res;
        if (c == '\\') {
            res = scan_escape(stream, mode);
        } else {
            res = c;
        }
        
        results.emplace_back(index, res);
        index++;
    }
}

void unescape_mixed(const std::string& src, Mode mode,
                    std::vector<std::pair<size_t, std::variant<MixedUnit, EscapeError>>>& results) {
    std::istringstream stream(src);
    size_t index = 0;

    while (stream.good()) {
        char c;
        if (!(stream >> c)) break;

        std::variant<MixedUnit, EscapeError> res = EscapeError::InvalidEscape;
        if (c == '\\') {
            auto escape_result = scan_escape(stream, mode);
            if (std::holds_alternative<char>(escape_result)) {
                res = MixedUnit(std::get<char>(escape_result));
            } else {
                res = std::get<EscapeError>(escape_result);
            }
        } else {
            res = MixedUnit(c);
        }

        results.emplace_back(index, res);
        index++;
    }
}
