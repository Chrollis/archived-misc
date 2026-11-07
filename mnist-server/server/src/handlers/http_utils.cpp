#include <handlers/http_utils.h>

#include <chrono>
#include <fstream>
#include <random>
#include <sstream>
#include <stdexcept>

namespace server {

void json_response(
    httplib::Response& res, const nlohmann::json& body, int status) {
    res.set_content(body.dump(), "application/json");
    res.status = status;
}

void json_error(httplib::Response& res, const std::string& msg, int status) {
    nlohmann::json err;
    err["success"] = false;
    err["error"] = msg;
    res.set_content(err.dump(), "application/json");
    res.status = status;
}

bool is_valid_dataset_name(const std::string& name) {
    return !name.empty() && name.find("..") == std::string::npos &&
           name.find('/') == std::string::npos &&
           name.find('\\') == std::string::npos;
}

std::vector<uint8_t> read_file_bytes(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) throw std::runtime_error("Cannot open file: " + path.string());
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<uint8_t> buffer(static_cast<size_t>(size));
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size))
        throw std::runtime_error("Cannot read file: " + path.string());
    return buffer;
}

std::string generate_temp_filename(
    const std::string& prefix, const std::string& extension) {
    auto now = std::chrono::steady_clock::now().time_since_epoch().count();
    std::mt19937_64 rng(static_cast<unsigned long long>(now));
    std::uniform_int_distribution<uint64_t> dist;
    uint64_t rand_part = dist(rng);

    std::ostringstream oss;
    oss << prefix << "_" << now << "_" << rand_part << extension;
    return oss.str();
}

}  // namespace server
