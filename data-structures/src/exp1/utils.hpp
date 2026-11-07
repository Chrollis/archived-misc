#pragma once
#include <format>
#include <regex>
#include <string>

namespace e1 {

using byte = unsigned char;

enum class token_t : byte {
    invalid_token = 0x00,
    number_token = 0x10,
    constant_number,
    binary_number,
    octal_number,
    hexadecimal_number,
    decimal_number,
    operator_token = 0x20,
    signal_operator,
    normal_operator,
    function_operator
};

inline byte operator&(token_t a, token_t b) noexcept {
    return static_cast<byte>(a) & static_cast<byte>(b);
}

namespace {
const std::string binary_pattern = R"((0b[01]+(\.[01]*)?))";
const std::string octal_pattern = R"((0o[0-7]+(\.[0-7]*)?))";
const std::string hexadecimal_pattern = R"((0x[0-9A-Fa-f]+(\.[0-9A-Fa-f]*)?))";
const std::string decimal_pattern = R"(((\d+\.?\d*|\.\d+)([eE][-+]?\d+)?))";
const std::string constant_pattern = R"(PI|E|PHI)";
const std::string normal_pattern = R"([+\-*/^()!%])";
const std::string signal_pattern = R"(pos|neg)";
const std::string function_pattern = R"(sin|cos|tan|cot|sec|csc|)"
                                     R"(arcsin|arccos|arctan|arccot|arcsec|arccsc|)"
                                     R"(ln|lg|deg|rad|sqrt|cbrt)";
}  // namespace

inline const std::string full_pattern() {
    return binary_pattern + "|" + octal_pattern + "|" + hexadecimal_pattern + "|" + decimal_pattern + "|" + constant_pattern + "|" + normal_pattern + "|" + function_pattern;
}

inline token_t token_type(const std::string& str) noexcept {
    if (std::regex_match(str, std::regex(binary_pattern))) {
        return token_t::binary_number;
    } else if (std::regex_match(str, std::regex(octal_pattern))) {
        return token_t::octal_number;
    } else if (std::regex_match(str, std::regex(hexadecimal_pattern))) {
        return token_t::hexadecimal_number;
    } else if (std::regex_match(str, std::regex(decimal_pattern))) {
        return token_t::decimal_number;
    } else if (std::regex_match(str, std::regex(normal_pattern))) {
        return token_t::normal_operator;
    } else if (std::regex_match(str, std::regex(constant_pattern))) {
        return token_t::constant_number;
    } else if (std::regex_match(str, std::regex(function_pattern))) {
        return token_t::function_operator;
    } else if (std::regex_match(str, std::regex(signal_pattern))) {
        return token_t::signal_operator;
    }
    return token_t::invalid_token;
}

inline bool is_operator(const std::string& str) noexcept {
    return token_t::operator_token & token_type(str);
}
inline bool is_function(const std::string& str) noexcept {
    return token_t::function_operator == token_type(str);
}
inline bool is_constant(const std::string& str) noexcept {
    return token_t::constant_number == token_type(str);
}
inline bool is_number(const std::string& str) noexcept {
    return token_t::number_token & token_type(str);
}

constexpr double CONSTANT_E = 2.718281828459;
constexpr double CONSTANT_PI = 3.1415926535898;
constexpr double CONSTANT_PHI = 0.61803398875;
constexpr byte PRIORITY_FUNCTION = 0xFF;

}  // namespace e1