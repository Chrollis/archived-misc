#include <handlers/http_utils.h>
#include <handlers/routes.h>

#include <core/config.h>
#include <core/job_state.h>
#include <core/train_manager.h>
#include <data/dataset_manager.h>

#include <filesystem>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>

namespace server {

void handle_train_start(const httplib::Request& req, httplib::Response& res) {
    try {
        auto body = nlohmann::json::parse(req.body);

        if (!body.contains("dataset_name") ||
            !body["dataset_name"].is_string()) {
            json_error(res, "Missing 'dataset_name' field");
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

        nlohmann::json hyperparams;
        if (body.contains("hyperparams") && body["hyperparams"].is_object()) {
            hyperparams = body["hyperparams"];
        }

        auto& cfg = config::instance();
        std::filesystem::path ds_dir = cfg.dataset_dir() / ds_name;
        std::filesystem::path img_path = ds_dir / "images.idx3-ubyte";
        std::filesystem::path lbl_path = ds_dir / "labels.idx1-ubyte";

        if (!std::filesystem::exists(img_path) ||
            !std::filesystem::exists(lbl_path)) {
            json_error(res, "Dataset files not found for: " + ds_name);
            return;
        }

        uint32_t total_samples =
            dataset_manager::instance().get_sample_count(img_path, lbl_path);
        if (offset >= total_samples) {
            json_error(res, "Offset exceeds dataset size", 400);
            return;
        }
        if (count == 0 || offset + count > total_samples) {
            count = total_samples - offset;
        }

        auto samples = dataset_manager::instance().load_dataset(
            img_path, lbl_path, static_cast<uint32_t>(offset),
            static_cast<uint32_t>(count));
        if (samples.empty()) {
            json_error(res, "Failed to load dataset or dataset is empty");
            return;
        }

        bool ok =
            train_manager::instance().start_training(samples, hyperparams);
        if (ok) {
            nlohmann::json resp;
            resp["success"] = true;
            resp["message"] = "Training started with " +
                              std::to_string(samples.size()) +
                              " samples (offset " + std::to_string(offset) +
                              ", count " + std::to_string(count) + ")";
            json_response(res, resp, 200);
        } else {
            json_error(
                res,
                "Failed to start training (maybe another training is already "
                "running)",
                409);
        }
    } catch (const std::exception& e) {
        json_error(res, std::string("Error: ") + e.what());
    }
}

void handle_train_cancel(const httplib::Request& req, httplib::Response& res) {
    train_manager::instance().cancel_training();
    nlohmann::json resp;
    resp["success"] = true;
    resp["message"] = "cancel requested";
    json_response(res, resp, 200);
}

void handle_train_status(const httplib::Request& req, httplib::Response& res) {
    auto status = train_manager::instance().status();

    nlohmann::json resp;
    resp["state"] = to_string(status.state);
    resp["current_epoch"] = status.current_epoch;
    resp["total_epochs"] = status.total_epochs;
    resp["processed_samples"] = status.processed_samples;
    resp["total_samples"] = status.total_samples;
    resp["current_loss"] = status.current_loss;
    resp["current_accuracy"] = status.current_accuracy;
    resp["loss_history"] = status.loss_history;
    resp["accuracy_history"] = status.accuracy_history;
    resp["error_message"] = status.error_message;

    json_response(res, resp, 200);
}

void handle_train_accept(const httplib::Request& req, httplib::Response& res) {
    try {
        auto body = nlohmann::json::parse(req.body);
        if (!body.contains("accept") || !body["accept"].is_boolean()) {
            json_error(
                res, "Missing or invalid 'accept' field (boolean required)");
            return;
        }
        bool accept = body["accept"].get<bool>();
        bool ok;
        if (accept) {
            ok = train_manager::instance().accept_trained_model();
        } else {
            ok = train_manager::instance().discard_trained_model();
        }

        nlohmann::json resp;
        if (ok) {
            resp["success"] = true;
            resp["message"] =
                accept ? "Trained model accepted and loaded into main model"
                       : "Trained model discarded";
            json_response(res, resp, 200);
        } else {
            resp["success"] = false;
            resp["error"] = accept
                                ? "No trained model available or accept failed"
                                : "No trained model available to discard";
            json_response(res, resp, 400);
        }
    } catch (const std::exception& e) {
        json_error(res, std::string("JSON parse error: ") + e.what());
    }
}

void register_train_routes(httplib::Server& svr) {
    svr.Post("/api/train/start", handle_train_start);
    svr.Post("/api/train/cancel", handle_train_cancel);
    svr.Get("/api/train/status", handle_train_status);
    svr.Post("/api/train/accept", handle_train_accept);
}

}  // namespace server
