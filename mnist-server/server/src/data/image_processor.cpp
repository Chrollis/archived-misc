#include <core/logger.h>
#include <core/model_manager.h>
#include <data/image_processor.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_WRITE_STATIC
#include <stb_image_write.h>

#include <string>
#include <utility>
#include <vector>

namespace server {

std::string base64_encode(const std::vector<uint8_t>& data) {
    static const char* encode_table =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";
    std::string result;
    for (size_t i = 0; i < data.size(); i += 3) {
        int a = data[i];
        int b = (i + 1 < data.size()) ? data[i + 1] : 0;
        int c = (i + 2 < data.size()) ? data[i + 2] : 0;
        result.push_back(encode_table[(a >> 2) & 0x3F]);
        result.push_back(encode_table[((a << 4) | (b >> 4)) & 0x3F]);
        result.push_back(
            (i + 1 < data.size()) ? encode_table[((b << 2) | (c >> 6)) & 0x3F]
                                  : '=');
        result.push_back((i + 2 < data.size()) ? encode_table[c & 0x3F] : '=');
    }
    return result;
}

image_processor& image_processor::instance() {
    static image_processor prc;
    return prc;
}

std::string image_processor::mat_to_mnist_base64(
    const Eigen::Matrix<uint8_t, 28, 28, Eigen::RowMajor>& mat) {
    std::vector<uint8_t> png;
    stbi_write_png_to_func(
        [](void* ctx, void* data, int size) {
            auto& vec = *reinterpret_cast<std::vector<uint8_t>*>(ctx);
            vec.insert(vec.end(), (uint8_t*)data, (uint8_t*)data + size);
        },
        &png, 28, 28, 1, mat.data(), 28);
    std::string b64 = base64_encode(png);
    return "data:image/png;base64," + b64;
}

std::pair<int, double> image_processor::predict_digit(
    const Eigen::Matrix<uint8_t, 28, 28, Eigen::RowMajor>& mat) {
    Eigen::MatrixXd dmat = core::reconstruct_to_matrixxd(mat);
    std::vector<std::vector<Eigen::MatrixXd>> input;
    input.push_back({dmat});
    auto results = model_manager::instance().predict(input);
    if (results.empty()) return {0, 0.0};
    return {results[0].first, results[0].second};
}

recognize_result image_processor::process_impl(
    const std::vector<uint8_t>& processed_img,
    int img_w,
    int img_h,
    const std::vector<Eigen::Matrix<uint8_t, 28, 28, Eigen::RowMajor>>&
        digit_mats,
    const std::vector<core::rectangle>& rects) {
    recognize_result result;

    for (size_t i = 0; i < digit_mats.size(); ++i) {
        recognized_digit digit;
        auto pred = predict_digit(digit_mats[i]);
        digit.digit = pred.first;
        digit.confidence = pred.second;
        digit.mnist_base64 = mat_to_mnist_base64(digit_mats[i]);
        if (i < rects.size()) digit.bbox = rects[i];
        result.digits.push_back(std::move(digit));
    }

    std::vector<uint8_t> png;
    stbi_write_png_to_func(
        [](void* ctx, void* data, int size) {
            auto& vec = *reinterpret_cast<std::vector<uint8_t>*>(ctx);
            vec.insert(vec.end(), (uint8_t*)data, (uint8_t*)data + size);
        },
        &png, img_w, img_h, 1, processed_img.data(), img_w);
    result.image_base64 = "data:image/png;base64," + base64_encode(png);
    return result;
}

nlohmann::json to_json(const recognize_result& result) {
    if (!result.success) {
        return {{"success", false}, {"error", result.error}};
    }

    nlohmann::json j;
    j["success"] = true;
    j["image_base64"] = result.image_base64;

    nlohmann::json digits_array = nlohmann::json::array();
    for (const auto& d : result.digits) {
        nlohmann::json item;
        item["digit"] = d.digit;
        item["confidence"] = d.confidence;
        item["bbox"] = {
            {"x", d.bbox.x}, {"y", d.bbox.y}, {"w", d.bbox.w}, {"h", d.bbox.h}};
        item["mnist_base64"] = d.mnist_base64;
        digits_array.push_back(std::move(item));
    }
    j["digits"] = std::move(digits_array);
    return j;
}

recognize_result image_processor::process_file(
    const std::filesystem::path& filepath) {
    auto& lgr = logger::instance();
    try {
        std::vector<Eigen::Matrix<uint8_t, 28, 28, Eigen::RowMajor>> digit_mats;
        std::vector<core::rectangle> rects;
        auto img_data =
            core::image::process_from_file(filepath, digit_mats, rects);
        core::image::sort_digits_by_reading_order(digit_mats, rects);
        return process_impl(
            img_data.px, img_data.w, img_data.h, digit_mats, rects);
    } catch (const std::exception& e) {
        lgr.error("process_file exception: " + std::string(e.what()));
        recognize_result result;
        result.success = false;
        result.error = e.what();
        return result;
    }
}

recognize_result image_processor::process_base64(
    const std::string& base64_str) {
    auto& lgr = logger::instance();
    try {
        std::vector<Eigen::Matrix<uint8_t, 28, 28, Eigen::RowMajor>> digit_mats;
        std::vector<core::rectangle> rects;
        auto img_data =
            core::image::process_from_base64(base64_str, digit_mats, rects);
        core::image::sort_digits_by_reading_order(digit_mats, rects);
        return process_impl(
            img_data.px, img_data.w, img_data.h, digit_mats, rects);
    } catch (const std::exception& e) {
        lgr.error("process_base64 exception: " + std::string(e.what()));
        recognize_result result;
        result.success = false;
        result.error = e.what();
        return result;
    }
}

}  // namespace server