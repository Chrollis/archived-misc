#pragma once

#include <httplib.h>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace server {

void json_response(
    httplib::Response& res, const nlohmann::json& body, int status = 200);

void json_error(
    httplib::Response& res, const std::string& msg, int status = 400);

bool is_valid_dataset_name(const std::string& name);

std::vector<uint8_t> read_file_bytes(const std::filesystem::path& path);

std::string generate_temp_filename(
    const std::string& prefix, const std::string& extension);

}  // namespace server
