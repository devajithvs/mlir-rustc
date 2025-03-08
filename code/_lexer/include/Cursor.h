#ifndef CURSOR_H
#define CURSOR_H

#include <string>
#include <functional>
#include <optional>

class Cursor {
private:
    std::string_view input;
    size_t position = 0;

public:
    explicit Cursor(std::string_view input) : input(input) {}

    inline char first() const {
        return position < input.size() ? input[position] : '\0';
    }

    inline char second() const {
        return (position + 1) < input.size() ? input[position + 1] : '\0';
    }

    inline char third() const {
        return (position + 2) < input.size() ? input[position + 2] : '\0';
    }

    inline bool is_eof() const {
        return position >= input.size();
    }

    inline std::optional<char> bump() {
        if (position < input.size()) {
            return input[position++];
        }
        return std::nullopt;  // Return an empty optional instead of void
    }    

    void eat_while(std::function<bool(char)> predicate) {
        while (!is_eof() && predicate(first())) {
            bump();
        }
    }

    void eat_until(char target) {
        while (!is_eof() && first() != target) {
            bump();
        }
    }

    // Store the previous character
    char prev() const {
        return (position == 0) ? '\0' : input[position - 1];
    }

    // Return the position of the current token
    uint32_t pos_within_token() const {
        return position;
    }

    // Reset position within a token
    void reset_pos_within_token() {
        position = 0;
    }

    std::optional<uint8_t> raw_double_quoted_string(uint32_t);


};

#endif // CURSOR_H
