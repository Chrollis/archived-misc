#pragma once

#include <core.h>
#include <atomic>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <nlohmann/json.hpp>
#include <shared_mutex>
#include <utility>
#include <vector>

namespace server {

class model_manager {
public:
    static model_manager& instance();

    model_manager(const model_manager&) = delete;
    model_manager& operator=(const model_manager&) = delete;
    model_manager(model_manager&&) = default;
    model_manager& operator=(model_manager&&) = default;

    bool load_from_bin(const std::filesystem::path& path);

    bool save_to_bin(const std::filesystem::path& path) const;

    bool init_from_json(const nlohmann::json& config);

    nlohmann::json export_to_json() const;

    std::vector<std::pair<uint8_t, double>> predict(
        const std::vector<std::vector<Eigen::MatrixXd>>& input);

    std::unique_ptr<core::model> clone_model() const;

    bool is_ready() const { return model_ready_; }

private:
    model_manager() = default;

    mutable std::shared_mutex mutex_;
    core::model model_;
    std::atomic<bool> model_ready_ = false;
};

}  // namespace server