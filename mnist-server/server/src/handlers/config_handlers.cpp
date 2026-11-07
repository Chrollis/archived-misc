#include <handlers/http_utils.h>
#include <handlers/routes.h>

#include <core/config.h>

#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>

namespace server {

void handle_get_config(const httplib::Request& req, httplib::Response& res) {
    auto& cfg = config::instance();
    nlohmann::json j;
    j["port"] = cfg.port();
    j["log_dir"] = cfg.log_dir().string();
    j["temp_dir"] = cfg.temp_dir().string();
    j["dataset_dir"] = cfg.dataset_dir().string();
    json_response(res, j, 200);
}

void handle_update_config(const httplib::Request& req, httplib::Response& res) {
    try {
        nlohmann::json body = nlohmann::json::parse(req.body);
        auto& cfg = config::instance();

        if (body.contains("port") && body["port"].is_number_integer()) {
            cfg.set_port(body["port"].get<int>());
        }
        if (body.contains("log_dir") && body["log_dir"].is_string()) {
            cfg.set_log_dir(body["log_dir"].get<std::string>());
        }
        if (body.contains("temp_dir") && body["temp_dir"].is_string()) {
            cfg.set_temp_dir(body["temp_dir"].get<std::string>());
        }
        if (body.contains("dataset_dir") && body["dataset_dir"].is_string()) {
            cfg.set_dataset_dir(body["dataset_dir"].get<std::string>());
        }

        cfg.save();

        nlohmann::json resp;
        resp["success"] = true;
        json_response(res, resp, 200);
    } catch (const std::exception& e) {
        json_error(res, std::string("Failed to update config: ") + e.what());
    }
}

void register_config_routes(httplib::Server& svr) {
    svr.Get("/api/config", handle_get_config);
    svr.Post("/api/config", handle_update_config);
}

}  // namespace server
