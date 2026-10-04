#pragma once

#include <filesystem>
#include <mutex>
#include <string>
#include <vector>

namespace server {

enum class log_level {
    debug,
    info,
    warning,
    error,
};

class logger {
public:
    static logger& instance();

    void set_level(log_level level);
    void set_log_dir(const std::filesystem::path& dir);

    void log(log_level level, const std::string& message);
    void debug(const std::string& msg) { log(log_level::debug, msg); }
    void info(const std::string& msg) { log(log_level::info, msg); }
    void warning(const std::string& msg) { log(log_level::warning, msg); }
    void error(const std::string& msg) { log(log_level::error, msg); }

    std::vector<std::string> get_logs(
        size_t count = 500, log_level min_level = log_level::debug) const;

private:
    logger();
    ~logger();

    static std::string level_to_string(log_level level);
    static std::string current_time();
    static std::string generate_filename();

    void flush_to_file(size_t from_count, size_t to_count);
    void flush_remaining();

    log_level level_ = log_level::info;
    std::filesystem::path log_dir_ = "./logs";

    mutable std::mutex mutex_;
    std::vector<std::string> ring_buffer_;
    size_t max_buffer_size_ = 500;
    size_t head_ = 0;
    size_t total_written_ = 0;
    size_t last_saved_count_ = 0;
};

}  // namespace server