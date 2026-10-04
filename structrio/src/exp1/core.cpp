#include "core.hpp"
#include <optional>

namespace e1 {

bool tokenizer::tokenize(const std::string& expr) {
    toks.clear();
    errs.clear();

    static std::regex pattern(full_pattern());
    uint64_t pos = 0;
    auto words_begin = std::sregex_iterator(expr.begin(), expr.end(), pattern);
    auto words_end = std::sregex_iterator();

    for (auto& it = words_begin; it != words_end; ++it) {
        std::smatch match = *it;
        std::string token = match.str();
        uint64_t tkpos = match.position();
        if (std::all_of(token.begin(), token.end(), ::isspace)) {
            continue;
        }

        if (tkpos > pos) {
            std::string unknown = expr.substr(pos, tkpos - pos);
            if (!std::all_of(unknown.begin(), unknown.end(), ::isspace)) {
                errs.push_back({unknown, "Unrecognized character or symbol"});
            }
        }

        toks.push_back(token);
        pos = tkpos + token.length();
    }

    if (pos < expr.length()) {
        std::string remaining = expr.substr(pos);
        if (!std::all_of(remaining.begin(), remaining.end(), isspace)) {
            errs.push_back({remaining, "Unrecognized characters at the end of the expression"});
        }
    }

    parse_signal_operators();
    return errs.empty();
}
bool tokenizer::validate(const std::string& expr) {
    if (!tokenize(expr)) {
        return 0;
    }
    parse_parenthese();
    parse_operator_sequence();
    parse_number_format();
    parse_function_usage();
    return errs.empty();
}
void tokenizer::parse_signal_operators() {
    std::vector<std::string> processed_tokens;
    for (size_t i = 0; i < toks.size(); ++i) {
        const std::string& token = toks[i];
        if (token == "+" || token == "-") {
            if (i == 0 || (i > 0 && (is_operator(toks[i - 1]) && toks[i - 1] != ")" && toks[i - 1] != "!") || is_function(toks[i - 1]))) {
                processed_tokens.push_back(token == "+" ? "pos" : "neg");
            } else {
                processed_tokens.push_back(token);
            }
        } else {
            processed_tokens.push_back(token);
        }
    }
    toks = processed_tokens;
}
void tokenizer::parse_parenthese() {
    std::stack<std::pair<std::string, size_t>> paren_stack;
    for (size_t i = 0; i < toks.size(); i++) {
        const auto& token = toks[i];
        if (token == "(") {
            paren_stack.push({token, i});
        } else if (token == ")") {
            if (paren_stack.empty()) {
                add_error(std::to_string(i), "Unmatched right parenthesis");
            } else {
                paren_stack.pop();
            }
        }
    }
    while (!paren_stack.empty()) {
        add_error(std::to_string(paren_stack.top().second), "Unmatched left parenthesis");
        paren_stack.pop();
    }
}
void tokenizer::parse_operator_sequence() {
    for (size_t i = 0; i < toks.size(); ++i) {
        const std::string& token = toks[i];
        if (token_type(token) == token_t::signal_operator) {
            if (i == toks.size() - 1) {
                add_error(std::to_string(i), "Expression ends with an operator");
            } else {
                if (i != 0 && token_type(toks[i - 1]) == token_t::signal_operator) {
                    add_error(std::to_string(i), "Expression contains consecutive sign operators");
                }
            }
        } else if (token == "!") {
            if (i == 0) {
                add_error(std::to_string(i), "Expression starts with a factorial operator");
            } else {
                const std::string& prev = toks[i - 1];
                if (!(is_number(prev) || prev == ")")) {
                    add_error(std::to_string(i), "Factorial must be preceded by a number, constant, or expression");
                }
            }
        } else if (token != "(" && token != ")" && token_type(token) == token_t::normal_operator) {
            if (i == 0) {
                add_error(std::to_string(i), "Expression starts with a binary operator");
            } else if (i == toks.size() - 1) {
                add_error(std::to_string(i), "Expression ends with an operator");
            } else if (token_type(toks[i - 1]) == token_t::signal_operator) {
                add_error(std::to_string(i), "Expression contains consecutive binary operators");
            }
        }
    }
}
void tokenizer::parse_number_format() {
    for (size_t i = 0; i < toks.size(); i++) {
        const auto& token = toks[i];
        if (is_number(token) && !is_constant(token)) {
            if (i > 0 && is_number(toks[i - 1])) {
                add_error(toks[i - 1] + token, "Expression contains consecutive numbers");
            } else {
                if ((token.find('e') != std::string::npos || token.find('E') != std::string::npos) && !token.starts_with("0x") && !token.starts_with("0o") && !token.starts_with("0b")) {
                    if (!std::regex_match(token, std::regex(decimal_pattern))) {
                        add_error(token, "Invalid scientific notation format");
                    }
                }

                if (token.starts_with("0b") && !std::regex_match(token, std::regex(binary_pattern))) {
                    add_error(token, "Invalid binary format");
                } else if (token.starts_with("0o") && !std::regex_match(token, std::regex(octal_pattern))) {
                    add_error(token, "Invalid octal format");
                } else if (token.starts_with("0x") && !std::regex_match(token, std::regex(hexadecimal_pattern))) {
                    add_error(token, "Invalid hexadecimal format");
                }
            }
        }
    }
}
void tokenizer::parse_function_usage() {
    for (size_t i = 0; i < toks.size(); ++i) {
        if (is_function(toks[i]) && (i + 1 >= toks.size() || toks[i + 1] != "(")) {
            add_error(toks[i], "Function name is not followed by a left parenthesis");
        }
    }
}
void tokenizer::add_error(const std::string& tok, std::string&& desc) {
    errs.push_back({tok, desc});
}
std::string tokenizer::verbose_info() const {
    std::string str;
    for (const auto& token : toks) {
        str += "[" + std::to_string(static_cast<byte>(token_type(token))) + "]: " + token + "\n";
    }
    for (const auto& error : errs) {
        str += "[" + error.first + "]: " + error.second + "\n";
    }
    while (!str.empty() && ::isspace(str.back())) str.pop_back();
    return str;
}

namespace {
std::optional<double> parse_number(const std::string& str) {
    if (!is_number(str)) return std::nullopt;
    token_t type = token_type(str);
    switch (type) {
        case token_t::decimal_number:
            return std::stod(str);
        case token_t::constant_number:
            if (str == "E") return CONSTANT_E;
            if (str == "PI") return CONSTANT_PI;
            if (str == "PHI") return CONSTANT_PHI;
            throw std::runtime_error("Encountered an invalid constant");
        default: {
            double value = 0;
            int radix = 10;
            std::string integer, fraction;
            switch (type) {
                case token_t::binary_number:
                    radix = 2;
                    break;
                case token_t::octal_number:
                    radix = 8;
                    break;
                case token_t::hexadecimal_number:
                    radix = 16;
                    break;
                default:
                    throw std::runtime_error("Encountered an invalid radix");
            }
            uint64_t point = str.find('.');
            if (point == std::string::npos) {
                integer = str.substr(2);
            } else {
                integer = str.substr(2, point - 2);
                fraction = str.substr(point + 1);
            }
            for (size_t i = 0; i < integer.length(); i++) {
                char t = integer[integer.length() - i - 1];
                value += pow(radix, i) * (t <= '9' ? t - '0' : t <= 'Z' ? t - 'A' + 10 : t - 'a' + 10);
            }
            for (size_t i = 0; i < fraction.length(); i++) {
                char t = fraction[i];
                value += pow(radix, -(int(i) + 1)) * (t <= '9' ? t - '0' : t <= 'Z' ? t - 'A' + 10 : t - 'a' + 10);
            }
            return value;
        }
    }
}
std::optional<token> parse_operator(const std::string& str) {
    static const std::unordered_map<std::string, token (*)(void)> operator_map = {
        {"+", []() { return add(); }},
        {"-", []() { return minus(); }},
        {"*", []() { return multiply(); }},
        {"/", []() { return divide(); }},
        {"%", []() { return modulo(); }},
        {"^", []() { return exponent(); }},
        {"!", []() { return factorial(); }},
        {"(", []() { return left_parentheses(); }},
        {")", []() { return right_parentheses(); }},
        {"sin", []() { return sine(); }},
        {"cos", []() { return cosine(); }},
        {"tan", []() { return tangent(); }},
        {"cot", []() { return cotangent(); }},
        {"sec", []() { return secant(); }},
        {"csc", []() { return cosecant(); }},
        {"arcsin", []() { return arcsine(); }},
        {"arccos", []() { return arccosine(); }},
        {"arctan", []() { return arctangent(); }},
        {"arccot", []() { return arccotangent(); }},
        {"arcsec", []() { return arcsecant(); }},
        {"arccsc", []() { return arccosecant(); }},
        {"lg", []() { return common_logarithm(); }},
        {"ln", []() { return natural_logarithm(); }},
        {"sqrt", []() { return square_root(); }},
        {"cbrt", []() { return cubic_root(); }},
        {"deg", []() { return degree(); }},
        {"rad", []() { return radian(); }},
    };
    auto it = operator_map.find(str);
    if (it != operator_map.end()) {
        return it->second();
    }
    return std::nullopt;
}
}  // namespace
token::token(const std::string& str) {
    std::string_view sv = str;
    while (!sv.empty() && ::isspace(sv.front())) sv = sv.substr(1);
    while (!sv.empty() && ::isspace(sv.back())) sv = sv.substr(0, sv.size() - 1);
    if (sv.empty()) {
        throw std::runtime_error("Encountered a blank token");
    }
    if (auto number = parse_number(std::string(sv))) {
        *this = token(*number);
    } else if (auto op_token = parse_operator(std::string(sv))) {
        *this = *op_token;
    } else
        throw std::runtime_error("Failed to parse token");
}

void expression::calculate(std::stack<token>& operands, const token& op) const {
    byte operand_num = op.operator_operand_num();
    if (operand_num == 0) {
        throw std::runtime_error("Encountered an operator with zero operands during evaluation");
    } else if (operand_num == 1) {
        double a = operands.top().number_value();
        operands.pop();
        operands.push(token(op.apply_operator(a, 0)));
    } else if (operand_num == 2) {
        double b = operands.top().number_value();
        operands.pop();
        double a = operands.top().number_value();
        operands.pop();
        operands.push(token(op.apply_operator(a, b)));
    } else {
        throw std::runtime_error("Encountered an operator with more than two operands during evaluation");
    }
}
expression::expression(const std::string& infix_expression) {
    tokenizer tokenizer;
    if (!tokenizer.validate(infix_expression)) {
        throw std::runtime_error("Invalid expression:\n" + tokenizer.verbose_info());
    }
    std::vector<std::string> strings = tokenizer.toks;
    for (const auto& str : strings) {
        infix.push_back(token(str));
    }
    std::stack<token> ops;
    for (const auto& tk : infix) {
        token_t type = tk.type;
        if (type == token_t::number_token) {
            postfix.push_back(tk);
        } else {
            if (tk.operator_symbol() == "(") {
                ops.push(tk);
            } else if (tk.operator_symbol() == ")") {
                while (!ops.empty()) {
                    if (ops.top().operator_symbol() == "(") {
                        ops.pop();
                        break;
                    } else {
                        postfix.push_back(ops.top());
                        ops.pop();
                    }
                }
            } else {
                while (!ops.empty() && ops.top().operator_prioriry() >= tk.operator_prioriry()) {
                    postfix.push_back(ops.top());
                    ops.pop();
                }
                ops.push(tk);
            }
        }
    }
    while (!ops.empty()) {
        postfix.push_back(ops.top());
        ops.pop();
    }
}
std::string expression::infix_expr() const {
    std::string str;
    for (const auto& tk : infix) {
        if (tk.type == token_t::number_token) {
            str += std::to_string(tk.number_value()) + ' ';
        } else {
            str += tk.operator_symbol() + ' ';
        }
    }
    return str;
}
std::string expression::postfix_expr() const {
    std::string str;
    for (const auto& tk : postfix) {
        if (tk.type == token_t::number_token) {
            str += std::to_string(tk.number_value()) + ' ';
        } else {
            str += tk.operator_symbol() + ' ';
        }
    }
    return str;
}

double expression::eval_postfix() const {
    std::stack<token> operands;
    for (const auto& tk : postfix) {
        if (tk.type == token_t::number_token) {
            operands.push(tk);
        } else {
            calculate(operands, tk);
        }
    }
    if (operands.size() != 1) {
        throw std::runtime_error("Evaluation finished with more than one operand on the stack");
    }
    return operands.top().number_value();
}

double expression::eval_infix() const {
    std::stack<token> operands;
    std::stack<token> ops;
    for (const auto& tk : infix) {
        if (tk.type == token_t::number_token) {
            operands.push(tk);
        } else {
            if (tk.operator_symbol() == "(") {
                ops.push(tk);
            } else if (tk.operator_symbol() == ")") {
                while (!ops.empty()) {
                    if (ops.top().operator_symbol() == "(") {
                        ops.pop();
                        break;
                    } else {
                        calculate(operands, ops.top());
                        ops.pop();
                    }
                }
            } else {
                while (!ops.empty() && ops.top().operator_prioriry() >= tk.operator_prioriry()) {
                    calculate(operands, ops.top());
                    ops.pop();
                }
                ops.push(tk);
            }
        }
    }
    while (!ops.empty()) {
        calculate(operands, ops.top());
        ops.pop();
    }
    if (operands.size() != 1) {
        throw std::runtime_error("Evaluation finished with more than one operand on the stack");
    }
    return operands.top().number_value();
}

}  // namespace e1
