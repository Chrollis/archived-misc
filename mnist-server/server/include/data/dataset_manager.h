#pragma once

#include <core.h>
#include <Eigen/Dense>
#include <cstdint>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <vector>

namespace server {

class dataset_manager {
public:
    static dataset_manager& instance();

    nlohmann::json save_samples(
        const std::vector<core::mnist_sample>& samples,
        const std::filesystem::path& images_path,
        const std::filesystem::path& labels_path,
        bool append = false);

    nlohmann::json merge_datasets(
        const std::vector<
            std::pair<std::filesystem::path, std::filesystem::path>>& files,
        const std::filesystem::path& output_images_path,
        const std::filesystem::path& output_labels_path);

    std::vector<core::mnist_sample> load_dataset(
        const std::filesystem::path& images_path,
        const std::filesystem::path& labels_path,
        uint32_t offset = 0,
        uint32_t count = 60000);

    uint32_t get_sample_count(
        const std::filesystem::path& images_path,
        const std::filesystem::path& labels_path);

private:
    dataset_manager() = default;
};

}  // namespace server