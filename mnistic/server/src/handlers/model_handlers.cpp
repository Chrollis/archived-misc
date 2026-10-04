#include <handlers/http_utils.h>
#include <handlers/routes.h>

#include <core/config.h>
#include <core/model_manager.h>

#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>
#include <vector>

namespace server {

void handle_model_load_bin(
    const httplib::Request& req, httplib::Response& res) {
    try {
        auto& files = req.form.files;
        auto it = files.find("model_file");
        if (it == files.end()) {
            json_error(res, "Missing 'model_file' field", 400);
            return;
        }
        const auto& model_file = it->second;
        if (model_file.content.empty()) {
            json_error(res, "Empty model file", 400);
            return;
        }

        auto& cfg = config::instance();
        std::filesystem::path temp_dir = cfg.temp_dir();
        std::filesystem::create_directories(temp_dir);

        std::string temp_filename =
            generate_temp_filename("model_load", ".bin");
        std::filesystem::path temp_path = temp_dir / temp_filename;

        {
            std::ofstream ofs(temp_path, std::ios::binary);
            ofs.write(model_file.content.data(), model_file.content.size());
        }

        bool ok = model_manager::instance().load_from_bin(temp_path);

        std::error_code ec;
        std::filesystem::remove(temp_path, ec);

        if (ok) {
            json_response(res, {{"success", true}}, 200);
        } else {
            json_error(res, "Failed to load model from file", 500);
        }
    } catch (const std::exception& e) {
        json_error(res, std::string("Error: ") + e.what(), 500);
    }
}

void handle_model_init_json(
    const httplib::Request& req, httplib::Response& res) {
    try {
        nlohmann::json config = nlohmann::json::parse(req.body);
        if (!config.contains("input_shape") ||
            !config.contains("num_classes") || !config.contains("layers")) {
            json_error(
                res,
                "Invalid model config: missing required fields (input_shape, "
                "num_classes, layers)");
            return;
        }
        bool ok = model_manager::instance().init_from_json(config);
        if (ok) {
            json_response(res, {{"success", true}}, 200);
        } else {
            json_error(res, "Failed to initialize model from JSON", 500);
        }
    } catch (const std::exception& e) {
        json_error(res, std::string("JSON parse error: ") + e.what());
    }
}

void handle_model_save_bin(
    const httplib::Request& req, httplib::Response& res) {
    if (!model_manager::instance().is_ready()) {
        json_error(res, "Model not ready, cannot save", 400);
        return;
    }

    auto& cfg = config::instance();
    std::filesystem::path temp_dir = cfg.temp_dir();
    std::filesystem::create_directories(temp_dir);

    std::string temp_filename = generate_temp_filename("model_save", ".bin");
    std::filesystem::path temp_path = temp_dir / temp_filename;

    bool ok = model_manager::instance().save_to_bin(temp_path);
    if (!ok) {
        json_error(res, "Failed to save model to temporary file", 500);
        return;
    }

    std::vector<uint8_t> buffer;
    try {
        buffer = read_file_bytes(temp_path);
    } catch (const std::exception& e) {
        std::error_code ec;
        std::filesystem::remove(temp_path, ec);
        json_error(
            res,
            std::string("Failed to read temporary model file: ") + e.what(),
            500);
        return;
    }

    std::error_code ec;
    std::filesystem::remove(temp_path, ec);

    std::string content(
        reinterpret_cast<const char*>(buffer.data()), buffer.size());

    res.set_header("Content-Disposition", "attachment; filename=\"model.bin\"");
    res.set_content(content, "application/octet-stream");
    res.status = 200;
}

void handle_model_export_json(
    const httplib::Request& req, httplib::Response& res) {
    nlohmann::json config = model_manager::instance().export_to_json();
    json_response(res, config, 200);
}

void handle_model_ready(const httplib::Request& req, httplib::Response& res) {
    bool ready = model_manager::instance().is_ready();
    json_response(res, {{"ready", ready}}, 200);
}

void register_model_routes(httplib::Server& svr) {
    svr.Post("/api/model/load_bin", handle_model_load_bin);
    svr.Post("/api/model/init_json", handle_model_init_json);
    svr.Get("/api/model/save_bin", handle_model_save_bin);
    svr.Get("/api/model/export_json", handle_model_export_json);
    svr.Get("/api/model/ready", handle_model_ready);
}

}  // namespace server
