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

struct evaluation_status {
    job_state state = job_state::idle;

    int processed_samples = 0;
    int total_samples = 0;
    double current_accuracy = 0.0;
    std::string error_message;
    double final_accuracy = 0.0;
    nlohmann::json per_class_accuracy;
};

class evaluate_manager {
public:
    static evaluate_manager& instance();

    evaluate_manager(const evaluate_manager&) = delete;
    evaluate_manager& operator=(const evaluate_manager&) = delete;
    evaluate_manager(evaluate_manager&&) = delete;
    evaluate_manager& operator=(evaluate_manager&&) = delete;

    bool start_evaluation(
        const std::string& dataset_name,
        size_t offset = 0,
        size_t count = 0,
        size_t batch_size = 32);

    void cancel_evaluation();

    evaluation_status status() const;

private:
    evaluate_manager() = default;
    ~evaluate_manager();

    void evaluation_thread_func(
        std::string dataset_name,
        size_t offset,
        size_t count,
        size_t batch_size);

    std::atomic<bool> cancel_requested_{false};
    std::unique_ptr<std::thread> worker_;
    mutable std::mutex status_mutex_;
    mutable evaluation_status status_;

    mutable double accum_correct_sum_ = 0.0;
    mutable size_t accum_count_ = 0;
};

}  // namespace server