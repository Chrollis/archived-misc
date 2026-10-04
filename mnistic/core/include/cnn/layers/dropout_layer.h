#pragma once

#include <cnn/layers/layer.h>
#include <random>
#include <vector>

namespace core {

class dropout_layer : public layer {
public:
    dropout_layer(
        io_shape in_shape = {1, 0, 0},
        double keep_prob = 0.5,
        bool training = true);

    std::vector<std::vector<Eigen::MatrixXd>> forward(
        const std::vector<std::vector<Eigen::MatrixXd>>& input) override;

    std::vector<std::vector<Eigen::MatrixXd>> backward(
        const std::vector<std::vector<Eigen::MatrixXd>>& grad_output,
        double learning_rate) override;

    void save(std::ostream& os) const override;
    void load(std::istream& is) override;

    double keep_prob() const { return keep_prob_; }

private:
    double keep_prob_;
    std::vector<std::vector<Eigen::MatrixXd>> masks_;

    static std::mt19937& rng();
};

}  // namespace core