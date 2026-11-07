#pragma once

#include <core.h>
#include <Eigen/Dense>
#include <cstdint>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <string>
#include <utility>
#include <vector>

namespace server {

std::string base64_encode(const std::vector<uint8_t>& data);

struct recognized_digit {
    int digit = 0;
    double confidence = 0.0;
    core::rectangle bbox{0, 0, 0, 0};
    std::string mnist_base64;
};

struct recognize_result {
    bool success = true;
    std::string error;
    std::string image_base64;
    std::vector<recognized_digit> digits;
};

nlohmann::json to_json(const recognize_result& result);

class image_processor {
public:
    static image_processor& instance();

    recognize_result process_file(const std::filesystem::path& filepath);

    recognize_result process_base64(const std::string& base64_str);

private:
    image_processor() = default;

    recognize_result process_impl(
        const std::vector<uint8_t>& processed_img,
        int img_w,
        int img_h,
        const std::vector<Eigen::Matrix<uint8_t, 28, 28, Eigen::RowMajor>>&
            digit_mats,
        const std::vector<core::rectangle>& rects);

    std::string mat_to_mnist_base64(
        const Eigen::Matrix<uint8_t, 28, 28, Eigen::RowMajor>& mat);

    std::pair<int, double> predict_digit(
        const Eigen::Matrix<uint8_t, 28, 28, Eigen::RowMajor>& mat);
};

}  // namespace server