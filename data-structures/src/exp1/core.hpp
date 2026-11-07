#pragma once
#include <cmath>
#include <functional>
#include <stack>
#include <variant>
#include "utils.hpp"
namespace e1 {

struct tokenizer {
    std::vector<std::string> toks;
    std::vector<std::pair<std::string, std::string>> errs;

    void parse_signal_operators();
    void parse_parenthese();
    void parse_operator_sequence();
    void parse_number_format();
    void parse_function_usage();
    void add_error(const std::string& tok, std::string&& desc);

    bool tokenize(const std::string& expr);
    bool validate(const std::string& expr);
    std::string verbose_info() const;
};

struct number_data {
    double value;
    number_data(double val = 0.0) : value(val) {}
};
struct operator_data {
    std::string symbol;
    byte operand_num;
    byte priority;
    std::function<double(double, double)> apply;

    operator_data(const std::string& sym = "", byte op_num = 0, byte pri = 0, std::function<double(double, double)> func = nullptr)
        : symbol(sym), operand_num(op_num), priority(pri), apply(std::move(func)) {}
};

struct token {
    token_t type;
    std::variant<number_data, operator_data> data;

    token(double val) : type(token_t::number_token), data(number_data{val}) {}
    token(const std::string& str);
    token(const std::string& sym, byte op_num, byte pri, std::function<double(double, double)> func) : type(token_t::operator_token), data(operator_data{sym, op_num, pri, std::move(func)}) {}
    bool is_number() const { return type == token_t::number_token; }
    bool is_operator() const { return type == token_t::operator_token; }
    bool is_valid() const { return type != token_t::invalid_token; }
    double number_value() const { return std::get<number_data>(data).value; }
    const std::string& operator_symbol() const { return std::get<operator_data>(data).symbol; }
    byte operator_operand_num() const { return std::get<operator_data>(data).operand_num; }
    byte operator_prioriry() const { return std::get<operator_data>(data).priority; }
    double apply_operator(double a, double b) const { return std::get<operator_data>(data).apply(a, b); }
    template <typename visitor>
    auto visit(visitor&& vis) -> decltype(auto) {
        return std::visit(std::forward<visitor>(vis), data);
    }
};

inline token add() {
    return token("+", 2, 1, [](double a, double b) { return a + b; });
}
inline token minus() {
    return token("-", 2, 1, [](double a, double b) { return a - b; });
}
inline token modulo() {
    return token("%", 2, 2, [](double a, double b) { return fmodl(a, b); });
}
inline token multiply() {
    return token("*", 2, 3, [](double a, double b) { return a * b; });
}
inline token divide() {
    return token("/", 2, 3, [](double a, double b) { return a / b; });
}
inline token posite() {
    return token("pos", 1, 4, [](double a, double b) { return a; });
}
inline token negate() {
    return token("neg", 1, 4, [](double a, double b) { return -a; });
}
inline token exponent() {
    return token("^", 2, 5, [](double a, double b) { return pow(a, b); });
}
inline token left_parentheses() {
    return token("(", 0, 0, [](double a, double b) { return 0; });
}
inline token right_parentheses() {
    return token(")", 0, 0, [](double a, double b) { return 0; });
}
inline token factorial() {
    return token("!", 1, 6, [](double a, double b) { return tgamma(a + 1); });
}
inline token sine() {
    return token("sin", 1, PRIORITY_FUNCTION, [](double a, double b) { return sin(a); });
}
inline token cosine() {
    return token("cos", 1, PRIORITY_FUNCTION, [](double a, double b) { return cos(a); });
}
inline token tangent() {
    return token("tan", 1, PRIORITY_FUNCTION, [](double a, double b) { return tan(a); });
}
inline token cotangent() {
    return token("cot", 1, PRIORITY_FUNCTION, [](double a, double b) { return 1 / tan(a); });
}
inline token secant() {
    return token("sec", 1, PRIORITY_FUNCTION, [](double a, double b) { return 1 / cos(a); });
}
inline token cosecant() {
    return token("csc", 1, PRIORITY_FUNCTION, [](double a, double b) { return 1 / sin(a); });
}
inline token arcsine() {
    return token("arcsin", 1, PRIORITY_FUNCTION, [](double a, double b) { return asin(a); });
}
inline token arccosine() {
    return token("arccos", 1, PRIORITY_FUNCTION, [](double a, double b) { return acos(a); });
}
inline token arctangent() {
    return token("arctan", 1, PRIORITY_FUNCTION, [](double a, double b) { return atan(a); });
}
inline token arccotangent() {
    return token("arccot", 1, PRIORITY_FUNCTION, [](double a, double b) { return atan(1 / a); });
}
inline token arcsecant() {
    return token("arcsec", 1, PRIORITY_FUNCTION, [](double a, double b) { return acos(1 / a); });
}
inline token arccosecant() {
    return token("arccsc", 1, PRIORITY_FUNCTION, [](double a, double b) { return asin(1 / a); });
}
inline token common_logarithm() {
    return token("lg", 1, PRIORITY_FUNCTION, [](double a, double b) { return log10(a); });
}
inline token natural_logarithm() {
    return token("ln", 1, PRIORITY_FUNCTION, [](double a, double b) { return log(a); });
}
inline token square_root() {
    return token("sqrt", 1, PRIORITY_FUNCTION, [](double a, double b) { return sqrt(a); });
}
inline token cubic_root() {
    return token("cbrt", 1, PRIORITY_FUNCTION, [](double a, double b) { return cbrt(a); });
}
inline token degree() {
    return token("deg", 1, PRIORITY_FUNCTION, [](double a, double b) { return a / CONSTANT_PI * 180; });
}
inline token radian() {
    return token("rad", 1, PRIORITY_FUNCTION, [](double a, double b) { return a / 180 * CONSTANT_PI; });
}

struct expression {
    std::vector<token> infix, postfix;

    expression(const std::string& expr);
    std::string infix_expr() const;
    std::string postfix_expr() const;
    double eval_postfix() const;
    double eval_infix() const;

    void calculate(std::stack<token>& operands, const token& op) const;
};
}  // namespace e1