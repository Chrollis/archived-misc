#pragma once

#include <cnn/models/sequential.h>
#include <cstdint>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace core {

class model {
public:
    model() = default;
    ~model() = default;

    model(const model&) = delete;
    model& operator=(const model&) = delete;
    model(model&&) = default;
    model& operator=(model&&) = default;

    static model from_json(const nlohmann::json& config);

    nlohmann::json to_json() const;

    std::vector<std::pair<uint8_t, double>> predict(
        const std::vector<std::vector<Eigen::MatrixXd>>& input);

    nlohmann::json train(
        const std::vector<std::vector<Eigen::MatrixXd>>& input,
        std::vector<uint8_t> labels,
        double learning_rate);

    double evaluate(
        const std::vector<std::vector<Eigen::MatrixXd>>& input,
        std::vector<uint8_t> labels);

    void save(std::filesystem::path filepath) const;
    void load(std::filesystem::path filepath);

    static nlohmann::json get_parameter_range(
        layer::layer_type type, const layer::io_shape& in_shape);

    static layer::io_shape compute_output_shape(
        layer::layer_type type,
        const layer::io_shape& in_shape,
        const nlohmann::json& params);

private:
    sequential net_;

    static double compute_loss(
        const std::vector<Eigen::MatrixXd>& probs,
        const std::vector<uint8_t>& labels);
    static std::vector<std::vector<Eigen::MatrixXd>> loss_gradient(
        const std::vector<Eigen::MatrixXd>& probs,
        const std::vector<uint8_t>& labels);

    static std::unique_ptr<layer> make_layer(
        layer::layer_type type,
        layer::io_shape in_shape,
        const nlohmann::json& params);
};

}  // namespace core