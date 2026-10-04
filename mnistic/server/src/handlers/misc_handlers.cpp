#include <handlers/http_utils.h>
#include <handlers/routes.h>

#include <core.h>
#include <core/config.h>
#include <core/logger.h>

#include <cctype>
#include <chrono>
#include <nlohmann/json.hpp>
#include <optional>
#include <stdexcept>
#include <string>

namespace server {

static std::string to_lower(const std::string& s) {
    std::string result = s;
    for (char& c : result)
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return result;
}

static std::optional<log_level> string_to_log_level(const std::string& s) {
    std::string lower = to_lower(s);
    if (lower == "debug") return log_level::debug;
    if (lower == "info") return log_level::info;
    if (lower == "warning") return log_level::warning;
    if (lower == "error") return log_level::error;
    return std::nullopt;
}

static std::optional<core::layer::layer_type> parse_layer_type(
    const std::string& type_str) {
    if (type_str == "convolve") return core::layer::layer_type::convolve;
    if (type_str == "pool") return core::layer::layer_type::pool;
    if (type_str == "flatten") return core::layer::layer_type::flatten;
    if (type_str == "dense") return core::layer::layer_type::dense;
    if (type_str == "dropout") return core::layer::layer_type::dropout;
    if (type_str == "batchnorm") return core::layer::layer_type::batchnorm;
    if (type_str == "activate") return core::layer::layer_type::activate;
    if (type_str == "softmax") return core::layer::layer_type::softmax;
    throw std::invalid_argument("Unknown layer type: " + type_str);
}

void handle_health(const httplib::Request& req, httplib::Response& res) {
    nlohmann::json resp;
    resp["status"] = "ok";
    json_response(res, resp, 200);
}

static auto start_time = std::chrono::steady_clock::now();

void handle_server_status(const httplib::Request& req, httplib::Response& res) {
    auto now = std::chrono::steady_clock::now();
    auto uptime =
        std::chrono::duration_cast<std::chrono::seconds>(now - start_time)
            .count();

    auto& cfg = config::instance();
    nlohmann::json resp;
    resp["uptime_seconds"] = uptime;
    json_response(res, resp, 200);
}

void handle_get_logs(const httplib::Request& req, httplib::Response& res) {
    int count = 100;
    if (req.has_param("count")) {
        std::string count_str = req.get_param_value("count");
        try {
            count = std::stoi(count_str);
            if (count < 1) count = 1;
            if (count > 1000) count = 1000;
        } catch (...) {
        }
    }

    log_level min_level = log_level::info;
    if (req.has_param("level")) {
        std::string level_str = req.get_param_value("level");
        auto opt_level = string_to_log_level(level_str);
        if (opt_level.has_value()) {
            min_level = opt_level.value();
        } else {
            json_error(
                res,
                "Invalid level parameter. Allowed: debug, info, warning, "
                "error");
            return;
        }
    }

    auto& lgr = logger::instance();
    std::vector<std::string> logs =
        lgr.get_logs(static_cast<size_t>(count), min_level);

    nlohmann::json response;
    response["logs"] = logs;
    response["success"] = true;
    json_response(res, response, 200);
}

void handle_layer_types(const httplib::Request& req, httplib::Response& res) {
    nlohmann::json result;
    result["types"] = {"convolve", "pool",      "flatten",  "dense",
                       "dropout",  "batchnorm", "activate", "softmax"};
    json_response(res, result, 200);
}

void handle_parameter_range(
    const httplib::Request& req, httplib::Response& res) {
    try {
        auto body = nlohmann::json::parse(req.body);
        if (!body.contains("type") || !body["type"].is_string()) {
            json_error(res, "Missing or invalid 'type' field", 400);
            return;
        }
        if (!body.contains("in_shape") || !body["in_shape"].is_array() ||
            body["in_shape"].size() != 3) {
            json_error(
                res, "Missing or invalid 'in_shape' field (must be [c, w, h])",
                400);
            return;
        }

        std::string type_str = body["type"].get<std::string>();
        auto in_shape_arr = body["in_shape"];
        int c = in_shape_arr[0].get<int>();
        int w = in_shape_arr[1].get<int>();
        int h = in_shape_arr[2].get<int>();
        if (c <= 0 || w <= 0 || h <= 0) {
            json_error(
                res, "Invalid dimensions in 'in_shape' (must be positive)",
                400);
            return;
        }
        core::layer::io_shape in_shape{c, w, h};

        auto ltype = parse_layer_type(type_str);
        if (!ltype.has_value()) {
            json_error(res, "Unknown layer type: " + type_str, 400);
            return;
        }

        nlohmann::json result =
            core::model::get_parameter_range(ltype.value(), in_shape);
        json_response(res, result, 200);
    } catch (const std::exception& e) {
        json_error(res, std::string("Error: ") + e.what(), 400);
    }
}

void handle_compute_output_shape(
    const httplib::Request& req, httplib::Response& res) {
    try {
        auto body = nlohmann::json::parse(req.body);
        if (!body.contains("type") || !body["type"].is_string()) {
            json_error(res, "Missing or invalid 'type' field", 400);
            return;
        }
        if (!body.contains("in_shape") || !body["in_shape"].is_array() ||
            body["in_shape"].size() != 3) {
            json_error(
                res, "Missing or invalid 'in_shape' field (must be [c, w, h])",
                400);
            return;
        }
        if (!body.contains("params") || !body["params"].is_object()) {
            json_error(
                res, "Missing or invalid 'params' field (must be an object)",
                400);
            return;
        }

        std::string type_str = body["type"].get<std::string>();
        auto in_shape_arr = body["in_shape"];
        int c = in_shape_arr[0].get<int>();
        int w = in_shape_arr[1].get<int>();
        int h = in_shape_arr[2].get<int>();
        if (c <= 0 || w <= 0 || h <= 0) {
            json_error(
                res, "Invalid dimensions in 'in_shape' (must be positive)",
                400);
            return;
        }

        core::layer::io_shape in_shape{c, w, h};
        auto ltype = parse_layer_type(type_str);
        if (!ltype.has_value()) {
            json_error(res, "Unknown layer type: " + type_str, 400);
            return;
        }

        const nlohmann::json& params = body["params"];

        core::layer::io_shape out_shape =
            core::model::compute_output_shape(ltype.value(), in_shape, params);

        nlohmann::json result;
        result["out_shape"] = {out_shape.c, out_shape.w, out_shape.h};
        json_response(res, result, 200);
    } catch (const std::invalid_argument& e) {
        json_error(res, e.what(), 400);
    } catch (const std::exception& e) {
        json_error(res, std::string("Error: ") + e.what(), 400);
    }
}

void handle_hyperparams_range(
    const httplib::Request& req, httplib::Response& res) {
    nlohmann::json result;
    result["learning_rate"] = {
        {"type", "double"}, {"default", 0.001}, {"min", 1e-6}, {"max", 0.1}};
    result["batch_size"] = {
        {"type", "int"}, {"default", 32}, {"min", 1}, {"max", 1024}};
    result["epochs"] = {
        {"type", "int"}, {"default", 10}, {"min", 1}, {"max", 1000}};
    json_response(res, result, 200);
}

void register_misc_routes(httplib::Server& svr) {
    svr.Get("/api/health", handle_health);
    svr.Get("/api/status", handle_server_status);
    svr.Get("/api/logs", handle_get_logs);

    svr.Get("/api/layer/types", handle_layer_types);
    svr.Post("/api/layer/parameter_range", handle_parameter_range);
    svr.Post("/api/layer/compute_output_shape", handle_compute_output_shape);

    svr.Get("/api/train/hyperparams_range", handle_hyperparams_range);
}

}  // namespace server
