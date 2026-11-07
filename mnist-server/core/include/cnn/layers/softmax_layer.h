#pragma once

#include <cnn/layers/layer.h>
#include <vector>

namespace core {

class softmax_layer : public layer {
public:
    softmax_layer(io_shape in_shape = {1, 0, 0}, bool training = true);

    std::vector<std::vector<Eigen::MatrixXd>> forward(
        const std::vector<std::vector<Eigen::MatrixXd>>& input) override;

    std::vector<std::vector<Eigen::MatrixXd>> backward(
        const std::vector<std::vector<Eigen::MatrixXd>>& grad_output,
        double learning_rate) override;

    void save(std::ostream& os) const override { save_header(os); }

    void load(std::istream& is) override { load_header(is); }

private:
    std::vector<Eigen::MatrixXd> cached_prob_;
};

}  // namespace core