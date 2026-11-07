#include <core/config.h>

#include <fstream>
#include <nlohmann/json.hpp>
#include <string>

namespace server {

config& config::instance() {
    static config cfg;
    return cfg;
}

void config::load(const std::filesystem::path& path) {
    std::ifstream f(path);
    if (!f.is_open()) {
        save(path);
        return;
    }
    try {
        nlohmann::json data = nlohmann::json::parse(f);
        if (data.contains("port")) port_ = data["port"].get<int>();
        if (data.contains("log_dir"))
            log_dir_ = data["log_dir"].get<std::string>();
        if (data.contains("temp_dir"))
            temp_dir_ = data["temp_dir"].get<std::string>();
        if (data.contains("dataset_dir"))
            dataset_dir_ = data["dataset_dir"].get<std::string>();
    } catch (const std::exception& e) {
        (void)e;
        save(path);
    }
}

void config::save(const std::filesystem::path& path) const {
    nlohmann::json data;
    data["port"] = port_;
    data["log_dir"] = log_dir_.string();
    data["temp_dir"] = temp_dir_.string();
    data["dataset_dir"] = dataset_dir_.string();

    std::filesystem::create_directories(path.parent_path());

    std::ofstream f(path);
    if (f.is_open()) {
        f << data.dump(4);
        f.close();
    }
}

}  // namespace server