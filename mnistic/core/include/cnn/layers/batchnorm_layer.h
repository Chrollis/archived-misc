#pragma once

#include <cnn/layers/layer.h>
#include <vector>

namespace core {

class batchnorm_layer : public layer {
public:
    batchnorm_layer(
        io_shape in_shape = {1, 0, 0},
        double eps = 1e-5,
        double momentum = 0.9,
        bool training = true);

    std::vector<std::vector<Eigen::MatrixXd>> forward(
        const std::vector<std::vector<Eigen::MatrixXd>>& input) override;

    std::vector<std::vector<Eigen::MatrixXd>> backward(
        const std::vector<std::vector<Eigen::MatrixXd>>& grad_output,
        double learning_rate) override;

    void save(std::ostream& os) const override;
    void load(std::istream& is) override;

    double eps() const { return eps_; }
    double momentum() const { return momentum_; }

private:
    double eps_;
    double momentum_;

    Eigen::VectorXd gamma_;
    Eigen::VectorXd beta_;

    Eigen::VectorXd running_mean_;
    Eigen::VectorXd running_var_;

    std::vector<std::vector<Eigen::MatrixXd>> cached_x_;
    Eigen::VectorXd cached_mean_;
    Eigen::VectorXd cached_var_;
    std::vector<std::vector<Eigen::MatrixXd>> cached_x_norm_;
};

}  // namespace core