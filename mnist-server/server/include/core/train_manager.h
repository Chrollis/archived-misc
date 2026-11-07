#pragma once

#include <core.h>
#include <core/job_state.h>
#include <atomic>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <mutex>
#include <nlohmann/json.hpp>
#include <thread>
#include <vector>

namespace server {

struct training_status {
    job_state state = job_state::idle;

    int current_epoch = 0;
    int total_epochs = 0;
    int processed_samples = 0;
    int total_samples = 0;
    double current_loss = 0.0;
    double current_accuracy = 0.0;
    std::vector<double> loss_history;
    std::vector<double> accuracy_history;
    std::string error_message;
};

class train_manager {
public:
    static train_manager& instance();

    train_manager(const train_manager&) = delete;
    train_manager& operator=(const train_manager&) = delete;
    train_manager(train_manager&&) = delete;
    train_manager& operator=(train_manager&&) = delete;

    bool start_training(
        const std::vector<core::mnist_sample>& samples,
        const nlohmann::json& hyperparams);

    void cancel_training();

    training_status status() const;

    bool accept_trained_model();

    bool discard_trained_model();

private:
    train_manager() = default;
    ~train_manager();

    void training_thread_func(
        const std::vector<core::mnist_sample>& samples,
        const nlohmann::json& hyperparams);

    std::atomic<bool> cancel_requested_{false};
    std::unique_ptr<std::thread> worker_;
    std::unique_ptr<core::model> trained_model_;

    mutable std::mutex status_mutex_;
    mutable training_status status_;

    mutable double accum_loss_sum_ = 0.0;
    mutable double accum_acc_sum_ = 0.0;
    mutable size_t accum_count_ = 0;
};

}  // namespace server