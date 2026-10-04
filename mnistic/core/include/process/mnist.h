#pragma once

#include <Eigen/Dense>
#include <cstdint>
#include <filesystem>
#include <vector>

namespace core {

struct mnist_sample {
    Eigen::Matrix<uint8_t, 28, 28, Eigen::RowMajor> img;
    uint8_t lbl;
};

namespace mnist {

uint32_t read_header_and_get_count(
    std::ifstream& img_file,
    std::ifstream& lbl_file,
    const std::filesystem::path& img_path,
    const std::filesystem::path& lbl_path);

std::vector<mnist_sample> read(
    const std::filesystem::path& img_path,
    const std::filesystem::path& lbl_path,
    uint32_t offset = 0,
    uint32_t count = 1024);

void write(
    const std::filesystem::path& img_path,
    const std::filesystem::path& lbl_path,
    const std::vector<mnist_sample>& samples);

}  // namespace mnist

}  // namespace core