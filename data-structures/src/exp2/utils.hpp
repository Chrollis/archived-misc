#pragma once
#include <format>
#include <string>

namespace e2 {
using byte = unsigned char;
inline std::string to_string(byte data) {
    return std::format("{:02X}", data);
}
}  // namespace e2