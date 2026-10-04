#pragma once

#include <Eigen/Dense>
#include <cstdint>
#include <string>
#include <vector>

namespace core {

struct rectangle {
    int x;
    int y;
    int w;
    int h;
};

uint32_t be32_to_host(uint32_t x) noexcept;

uint32_t host_to_be32(uint32_t x) noexcept;

uint64_t be64_to_host(uint64_t x) noexcept;

uint64_t host_to_be64(uint64_t x) noexcept;

double bedb_to_host(double x) noexcept;

double host_to_bedb(double x) noexcept;

std::vector<uint8_t> base64_decode(const std::string& base64_str);

class union_find {
public:
    union_find() = default;

    int add();

    int find(int x);

    void unite(int x, int y);

private:
    std::vector<int> parent;
    std::vector<int> rank;
};

Eigen::MatrixXd reconstruct_to_matrixxd(
    const Eigen::Matrix<uint8_t, 28, 28, Eigen::RowMajor>& src);

}  // namespace core
