#pragma once

#include <cstddef>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

namespace ui {
inline constexpr std::string_view reset = "\x1b[0m";
inline constexpr std::string_view bold = "\x1b[1m";
inline constexpr std::string_view dim = "\x1b[2m";

inline constexpr std::string_view red = "\x1b[31m";
inline constexpr std::string_view green = "\x1b[32m";
inline constexpr std::string_view yellow = "\x1b[33m";
inline constexpr std::string_view blue = "\x1b[34m";
inline constexpr std::string_view magenta = "\x1b[35m";
inline constexpr std::string_view cyan = "\x1b[36m";

inline void info(std::string_view m) {
    std::cout << cyan << "[*] " << reset << m << '\n';
}
inline void success(std::string_view m) {
    std::cout << green << "[+] " << reset << m << '\n';
}
inline void warning(std::string_view m) {
    std::cout << yellow << "[!] " << reset << m << '\n';
}
inline void error(std::string_view m) {
    std::cout << red << "[-] " << reset << m << '\n';
}

inline void print_title(std::string_view title) {
    std::cout << cyan << bold << "\n"
              << "+--------------------------------------+\n"
              << "| " << title << "\n"
              << "+--------------------------------------+\n"
              << reset;
}

inline void print_separator(std::size_t n = 40) {
    std::cout << dim << std::string(n, '-') << reset << '\n';
}

inline std::string prompt(std::string_view q, std::string_view def = {}) {
    std::cout << cyan << "? " << reset << q;
    if (!def.empty()) std::cout << " " << dim << '[' << def << ']' << reset;
    std::cout << " " << std::flush;
    std::string line;
    if (!std::getline(std::cin, line)) return std::string(def);
    if (line.empty()) return std::string(def);
    std::cout << dim << "<- " + line << reset << "\n";
    return line;
}

inline bool confirm(std::string_view q, bool default_yes = false) {
    std::cout << yellow << "? " << reset << q << " " << dim << (default_yes ? "(Y/n)" : "(y/N)") << reset << " " << std::flush;
    std::string line;
    if (!std::getline(std::cin, line)) return default_yes;
    if (line.empty()) return default_yes;
    return line[0] == 'y' || line[0] == 'Y';
}

inline int select(std::string_view q, const std::vector<std::string>& options) {
    if (options.empty()) return -1;
    std::cout << magenta << "? " << reset << q << '\n';
    for (std::size_t i = 0; i < options.size(); ++i) std::cout << "  " << cyan << (i + 1) << ")" << reset << " " << options[i] << '\n';
    for (;;) {
        std::cout << dim << "  choice [1-" << options.size() << "]: " << reset << std::flush;
        std::string line;
        if (!std::getline(std::cin, line)) return -1;
        try {
            std::size_t pos = 0;
            int v = std::stoi(line, &pos);
            if (pos == line.size() && v >= 1 && v <= static_cast<int>(options.size())) return v - 1;
        } catch (...) {
        }
        error("invalid input, try again");
    }
}

inline void pause(std::string_view msg = "Press Enter to continue...") {
    std::cout << dim << msg << reset << std::flush;
    std::string line;
    std::getline(std::cin, line);
}

}  // namespace ui