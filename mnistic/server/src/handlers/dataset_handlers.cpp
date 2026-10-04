#include <handlers/http_utils.h>
#include <handlers/routes.h>

#include <core.h>
#include <core/config.h>
#include <data/dataset_manager.h>
#include <data/image_processor.h>

#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>
#include <vector>

namespace server {

void handle_list_datasets(const httplib::Request& req, httplib::Response& res) {
    auto& cfg = config::instance();
    std::filesystem::path dataset_root = cfg.dataset_dir();

    nlohmann::json result;
    result["datasets"] = nlohmann::json::array();

    if (!std::filesystem::exists(dataset_root) ||
        !std::filesystem::is_directory(dataset_root)) {
        json_error(
            res, "Dataset directory does not exist or is not a directory", 500);
        return;
    }

    for (const auto& entry :
         std::filesystem::directory_iterator(dataset_root)) {
        if (!entry.is_directory()) continue;

        std::string ds_name = entry.path().filename().string();
        std::filesystem::path images_path = entry.path() / "images.idx3-ubyte";
        std::filesystem::path labels_path = entry.path() / "labels.idx1-ubyte";

        nlohmann::json ds_info;
        ds_info["name"] = ds_name;

        if (!std::filesystem::exists(images_path) ||
            !std::filesystem::exists(labels_path)) {
            ds_info["sample_count"] = nullptr;
            ds_info["error"] = "Missing images.idx3-ubyte or labels.idx1-ubyte";
        } else {
            try {
                uint32_t count = dataset_manager::instance().get_sample_count(
                    images_path, labels_path);
                ds_info["sample_count"] = count;
                ds_info["error"] = nullptr;
            } catch (const std::exception& e) {
                ds_info["sample_count"] = nullptr;
                ds_info["error"] =
                    std::string("Failed to read dataset: ") + e.what();
            }
        }

        result["datasets"].push_back(ds_info);
    }

    json_response(res, result, 200);
}

void handle_upload_dataset(
    const httplib::Request& req, httplib::Response& res) {
    struct TempFiles {
        std::filesystem::path images, labels;
        TempFiles(const std::filesystem::path& dir, const std::string& base)
            : images(dir / (base + "_images.tmp")),
              labels(dir / (base + "_labels.tmp")) {}
        ~TempFiles() {
            std::error_code ec;
            std::filesystem::remove(images, ec);
            std::filesystem::remove(labels, ec);
        }
    };

    try {
        if (!req.has_param("name")) {
            json_error(res, "Missing 'name' query parameter", 400);
            return;
        }
        std::string ds_name = req.get_param_value("name");
        if (!is_valid_dataset_name(ds_name)) {
            json_error(res, "Invalid dataset name", 400);
            return;
        }

        auto& files = req.form.files;
        auto images_it = files.find("images_file");
        auto labels_it = files.find("labels_file");
        if (images_it == files.end() || labels_it == files.end()) {
            json_error(res, "Missing images_file or labels_file", 400);
            return;
        }
        const auto& images_file = images_it->second;
        const auto& labels_file = labels_it->second;

        auto& cfg = config::instance();
        std::filesystem::path temp_dir = cfg.temp_dir() / "dataset_upload_tmp";
        std::filesystem::create_directories(temp_dir);
        TempFiles tmp(temp_dir, ds_name);

        {
            std::ofstream ofs(tmp.images, std::ios::binary);
            ofs.write(images_file.content.data(), images_file.content.size());
        }
        {
            std::ofstream ofs(tmp.labels, std::ios::binary);
            ofs.write(labels_file.content.data(), labels_file.content.size());
        }

        uint32_t sample_count = 0;
        try {
            sample_count = dataset_manager::instance().get_sample_count(
                tmp.images, tmp.labels);
        } catch (const std::exception& e) {
            json_error(
                res, std::string("Dataset validation failed: ") + e.what(),
                400);
            return;
        }

        std::filesystem::path final_dir = cfg.dataset_dir() / ds_name;
        std::filesystem::create_directories(final_dir);
        std::filesystem::path final_images = final_dir / "images.idx3-ubyte";
        std::filesystem::path final_labels = final_dir / "labels.idx1-ubyte";

        std::error_code ec;
        std::filesystem::rename(tmp.images, final_images, ec);
        if (ec) {
            json_error(res, "Failed to save images file: " + ec.message(), 500);
            return;
        }
        std::filesystem::rename(tmp.labels, final_labels, ec);
        if (ec) {
            std::filesystem::remove(final_images, ec);
            json_error(res, "Failed to save labels file: " + ec.message(), 500);
            return;
        }

        nlohmann::json resp;
        resp["success"] = true;
        resp["dataset_name"] = ds_name;
        resp["sample_count"] = sample_count;
        json_response(res, resp, 200);
    } catch (const std::exception& e) {
        json_error(res, std::string("Upload error: ") + e.what(), 500);
    }
}

void handle_download_dataset(
    const httplib::Request& req, httplib::Response& res) {
    if (!req.has_param("name")) {
        json_error(res, "Missing 'name' query parameter", 400);
        return;
    }
    std::string ds_name = req.get_param_value("name");
    if (!is_valid_dataset_name(ds_name)) {
        json_error(res, "Invalid dataset name", 400);
        return;
    }

    auto& cfg = config::instance();
    std::filesystem::path ds_dir = cfg.dataset_dir() / ds_name;
    std::filesystem::path img_path = ds_dir / "images.idx3-ubyte";
    std::filesystem::path lbl_path = ds_dir / "labels.idx1-ubyte";

    if (!std::filesystem::exists(img_path) ||
        !std::filesystem::exists(lbl_path)) {
        json_error(res, "Dataset not found or missing files", 404);
        return;
    }

    try {
        auto img_data = read_file_bytes(img_path);
        auto lbl_data = read_file_bytes(lbl_path);

        uint32_t sample_count =
            dataset_manager::instance().get_sample_count(img_path, lbl_path);

        nlohmann::json result;
        result["name"] = ds_name;
        result["sample_count"] = sample_count;
        result["images_base64"] = base64_encode(img_data);
        result["labels_base64"] = base64_encode(lbl_data);

        json_response(res, result, 200);
    } catch (const std::exception& e) {
        json_error(
            res, std::string("Failed to read dataset: ") + e.what(), 500);
    }
}

void handle_delete_dataset(
    const httplib::Request& req, httplib::Response& res) {
    try {
        auto body = nlohmann::json::parse(req.body);
        if (!body.contains("name") || !body["name"].is_string()) {
            json_error(res, "Missing or invalid 'name' field", 400);
            return;
        }
        std::string ds_name = body["name"].get<std::string>();
        if (!is_valid_dataset_name(ds_name)) {
            json_error(res, "Invalid dataset name", 400);
            return;
        }

        auto& cfg = config::instance();
        std::filesystem::path ds_dir = cfg.dataset_dir() / ds_name;
        if (!std::filesystem::exists(ds_dir)) {
            json_error(res, "Dataset not found", 404);
            return;
        }
        if (!std::filesystem::is_directory(ds_dir)) {
            json_error(res, "Path is not a directory", 400);
            return;
        }

        std::error_code ec;
        std::filesystem::remove_all(ds_dir, ec);
        if (ec) {
            json_error(res, "Failed to delete dataset: " + ec.message(), 500);
            return;
        }

        nlohmann::json resp;
        resp["success"] = true;
        resp["message"] = "Dataset '" + ds_name + "' deleted";
        json_response(res, resp, 200);
    } catch (const std::exception& e) {
        json_error(res, std::string("Error: ") + e.what(), 400);
    }
}

void handle_save_samples(const httplib::Request& req, httplib::Response& res) {
    try {
        auto body = nlohmann::json::parse(req.body);

        if (!body.contains("dataset_name") ||
            !body["dataset_name"].is_string() || !body.contains("samples") ||
            !body["samples"].is_array()) {
            json_error(res, "Missing required fields: dataset_name, samples");
            return;
        }

        std::string ds_name = body["dataset_name"].get<std::string>();
        if (!is_valid_dataset_name(ds_name)) {
            json_error(res, "Invalid dataset name", 400);
            return;
        }

        bool append = body.value("append", false);

        auto& cfg = config::instance();
        std::filesystem::path ds_dir = cfg.dataset_dir() / ds_name;
        std::filesystem::create_directories(ds_dir);

        std::filesystem::path img_path = ds_dir / "images.idx3-ubyte";
        std::filesystem::path lbl_path = ds_dir / "labels.idx1-ubyte";

        std::vector<core::mnist_sample> mnist_samples;
        for (const auto& item : body["samples"]) {
            if (!item.contains("image_base64") || !item.contains("label")) {
                json_error(
                    res, "Each sample must have 'image_base64' and 'label'");
                return;
            }
            std::string b64 = item["image_base64"].get<std::string>();
            int label = item["label"].get<int>();
            if (label < 0 || label > 9) {
                json_error(res, "Label must be between 0 and 9");
                return;
            }
            try {
                auto sample = core::image::png_base64_to_mnist_sample(
                    b64, static_cast<uint8_t>(label));
                mnist_samples.push_back(sample);
            } catch (const std::exception& e) {
                json_error(
                    res, std::string("Failed to decode image: ") + e.what());
                return;
            }
        }

        auto result = dataset_manager::instance().save_samples(
            mnist_samples, img_path, lbl_path, append);

        json_response(res, result, result.value("success", false) ? 200 : 500);
    } catch (const std::exception& e) {
        json_error(res, std::string("JSON parse error: ") + e.what());
    }
}

void handle_merge_datasets(
    const httplib::Request& req, httplib::Response& res) {
    try {
        auto body = nlohmann::json::parse(req.body);
        if (!body.contains("source_datasets") ||
            !body["source_datasets"].is_array() ||
            !body.contains("target_dataset") ||
            !body["target_dataset"].is_string()) {
            json_error(
                res,
                "Missing required fields: source_datasets (array), "
                "target_dataset (string)");
            return;
        }

        std::string target_name = body["target_dataset"].get<std::string>();
        if (!is_valid_dataset_name(target_name)) {
            json_error(res, "Invalid target dataset name", 400);
            return;
        }

        auto& cfg = config::instance();
        std::filesystem::path target_dir = cfg.dataset_dir() / target_name;
        std::filesystem::path out_img = target_dir / "images.idx3-ubyte";
        std::filesystem::path out_lbl = target_dir / "labels.idx1-ubyte";

        std::vector<std::pair<std::filesystem::path, std::filesystem::path>>
            pairs;
        for (const auto& ds_name : body["source_datasets"]) {
            if (!ds_name.is_string()) {
                json_error(res, "Each source dataset name must be a string");
                return;
            }
            std::string name = ds_name.get<std::string>();
            if (!is_valid_dataset_name(name)) {
                json_error(res, "Invalid source dataset name: " + name, 400);
                return;
            }
            std::filesystem::path src_dir = cfg.dataset_dir() / name;
            std::filesystem::path img = src_dir / "images.idx3-ubyte";
            std::filesystem::path lbl = src_dir / "labels.idx1-ubyte";
            if (!std::filesystem::exists(img) ||
                !std::filesystem::exists(lbl)) {
                json_error(res, "Source dataset missing files: " + name, 400);
                return;
            }
            pairs.emplace_back(img, lbl);
        }

        if (pairs.empty()) {
            json_error(res, "No valid source datasets provided", 400);
            return;
        }

        std::filesystem::create_directories(target_dir);

        auto result =
            dataset_manager::instance().merge_datasets(pairs, out_img, out_lbl);
        json_response(res, result, result.value("success", false) ? 200 : 500);
    } catch (const std::exception& e) {
        json_error(res, std::string("JSON parse error: ") + e.what());
    }
}

void register_dataset_routes(httplib::Server& svr) {
    svr.Get("/api/datasets", handle_list_datasets);
    svr.Post("/api/datasets/upload", handle_upload_dataset);
    svr.Get("/api/datasets/download", handle_download_dataset);
    svr.Post("/api/datasets/delete", handle_delete_dataset);
    svr.Post("/api/datasets/save_samples", handle_save_samples);
    svr.Post("/api/datasets/merge", handle_merge_datasets);
}

}  // namespace server
