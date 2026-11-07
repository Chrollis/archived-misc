#pragma once

#include <filesystem>

namespace server {

class config {
public:
    static config& instance();

    void load(const std::filesystem::path& path = "./config.json");

    void save(const std::filesystem::path& path = "./config.json") const;

    int port() const { return port_; }
    void set_port(int port) { port_ = port; }

    const std::filesystem::path& log_dir() const { return log_dir_; }
    void set_log_dir(const std::filesystem::path& dir) { log_dir_ = dir; }

    const std::filesystem::path& temp_dir() const { return temp_dir_; }
    void set_temp_dir(const std::filesystem::path& dir) { temp_dir_ = dir; }

    const std::filesystem::path& dataset_dir() const { return dataset_dir_; }
    void set_dataset_dir(const std::filesystem::path& dir) {
        dataset_dir_ = dir;
    }

private:
    config() = default;

    int port_ = 3705;
    std::filesystem::path log_dir_ = "./logs";
    std::filesystem::path temp_dir_ = "./temp";
    std::filesystem::path dataset_dir_ = "./dataset";
};

}  // namespace server