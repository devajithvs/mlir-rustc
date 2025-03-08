#ifndef CURSOR_H
#define CURSOR_H

#include <string>
#include <functional>

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

    inline void bump() {
        if (position < input.size()) position++;
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
};

#endif // CURSOR_H
