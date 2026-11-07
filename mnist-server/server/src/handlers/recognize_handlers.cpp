#include <handlers/http_utils.h>
#include <handlers/routes.h>

#include <data/image_processor.h>

#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>

namespace server {

void handle_recognize(const httplib::Request& req, httplib::Response& res) {
    try {
        auto body = nlohmann::json::parse(req.body);
        if (!body.contains("base64") || !body["base64"].is_string()) {
            json_error(res, "Missing or invalid 'base64' field");
            return;
        }
        std::string base64_str = body.value("base64", "");
        if (base64_str.empty()) {
            json_error(res, "Empty base64 string");
            return;
        }

        auto result = image_processor::instance().process_base64(base64_str);
        if (result.success) {
            json_response(res, to_json(result), 200);
        } else {
            json_error(res, result.error, 500);
        }
    } catch (const std::exception& e) {
        json_error(res, std::string("JSON parse error: ") + e.what());
    }
}

void register_recognize_routes(httplib::Server& svr) {
    svr.Post("/api/recognize", handle_recognize);
}

}  // namespace server
