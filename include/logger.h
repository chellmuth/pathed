#pragma once

#include <iostream>
#include <ostream>
#include <string>

namespace Logger {
    void line(const std::string &line);

    struct LoggerCout {};
    extern LoggerCout cout;
    extern const bool enabled;

    template <typename T>
    inline LoggerCout& operator<< (LoggerCout &s, const T &x) {
        if (enabled) {
            std::cout << x;
        }
        return s;
    }

    inline LoggerCout& operator<< (LoggerCout &s, std::ostream& (*f)(std::ostream &)) {
        if (enabled) {
            f(std::cout);
        }
        return s;
    }

    inline LoggerCout& operator<< (LoggerCout &s, std::ostream& (*f)(std::ios &)) {
        if (enabled) {
            f(std::cout);
        }
        return s;
    }

    inline LoggerCout& operator<< (LoggerCout &s, std::ostream& (*f)(std::ios_base &)) {
        if (enabled) {
            f(std::cout);
        }
        return s;
    }
}
