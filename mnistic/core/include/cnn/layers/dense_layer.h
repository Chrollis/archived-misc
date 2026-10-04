#pragma once

#include <cnn/layers/layer.h>
#include <random>
#include <vector>

namespace core {

class dense_layer : public layer {
public:
    enum class init_type {
        gaussian,
        xavier,
        he,
        load,
    };

    dense_layer(
        io_shape in_shape = {1, 0, 0},
        io_shape out_shape = {1, 0, 0},
        init_type init = init_type::he,
        bool training = true);

    std::vector<std::vector<Eigen::MatrixXd>> forward(
        const std::vector<std::vector<Eigen::MatrixXd>>& input) override;

    std::vector<std::vector<Eigen::MatrixXd>> backward(
        const std::vector<std::vector<Eigen::MatrixXd>>& grad_output,
        double learning_rate) override;

    void save(std::ostream& os) const override;
    void load(std::istream& is) override;

private:
    Eigen::MatrixXd weights_;
    Eigen::MatrixXd biases_;

    std::vector<std::vector<Eigen::MatrixXd>> cached_input_;

    static std::mt19937& rng();

    void init_weights(init_type type);
};

}  // namespace core