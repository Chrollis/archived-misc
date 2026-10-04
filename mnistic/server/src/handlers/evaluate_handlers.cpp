#include <handlers/http_utils.h>
#include <handlers/routes.h>

#include <core/evaluate_manager.h>
#include <core/job_state.h>

#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>

namespace server {

void handle_evaluate_start(
    const httplib::Request& req, httplib::Response& res) {
    try {
        auto body = nlohmann::json::parse(req.body);
        if (!body.contains("dataset_name") ||
            !body["dataset_name"].is_string()) {
            json_error(res, "Missing or invalid 'dataset_name' field");
            return;
        }
        std::string ds_name = body["dataset_name"].get<std::string>();
        if (!is_valid_dataset_name(ds_name)) {
            json_error(res, "Invalid dataset name", 400);
            return;
        }

        size_t offset = 0;
        if (body.contains("offset") && body["offset"].is_number_unsigned()) {
            offset = body["offset"].get<size_t>();
        }
        size_t count = 0;
        if (body.contains("count") && body["count"].is_number_unsigned()) {
            count = body["count"].get<size_t>();
        }
        size_t batch_size = 32;
        if (body.contains("batch_size") &&
            body["batch_size"].is_number_unsigned()) {
            batch_size = body["batch_size"].get<size_t>();
            if (batch_size == 0) batch_size = 1;
        }

        bool ok = evaluate_manager::instance().start_evaluation(
            ds_name, offset, count, batch_size);
        if (ok) {
            json_response(
                res, {{"success", true}, {"message", "Evaluation started"}},
                200);
        } else {
            json_error(
                res,
                "Failed to start evaluation (maybe another evaluation is "
                "already running)",
                409);
        }
    } catch (const std::exception& e) {
        json_error(res, std::string("JSON parse error: ") + e.what());
    }
}

void handle_evaluate_cancel(
    const httplib::Request& req, httplib::Response& res) {
    evaluate_manager::instance().cancel_evaluation();
    nlohmann::json resp;
    resp["success"] = true;
    resp["message"] = "cancel requested";
    json_response(res, resp, 200);
}

void handle_evaluate_status(
    const httplib::Request& req, httplib::Response& res) {
    auto status = evaluate_manager::instance().status();
    nlohmann::json resp;

    resp["state"] = to_string(status.state);
    resp["processed_samples"] = status.processed_samples;
    resp["total_samples"] = status.total_samples;
    resp["current_accuracy"] = status.current_accuracy;
    resp["error_message"] = status.error_message;

    if (status.state == job_state::finished) {
        resp["final_accuracy"] = status.final_accuracy;
        resp["per_class_accuracy"] = status.per_class_accuracy;
    }

    json_response(res, resp, 200);
}

void register_evaluate_routes(httplib::Server& svr) {
    svr.Post("/api/evaluate/start", handle_evaluate_start);
    svr.Post("/api/evaluate/cancel", handle_evaluate_cancel);
    svr.Get("/api/evaluate/status", handle_evaluate_status);
}

}  // namespace server
