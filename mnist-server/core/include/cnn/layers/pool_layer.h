#pragma once

#include <cnn/layers/layer.h>
#include <cstdint>
#include <vector>

namespace core {

class pool_layer : public layer {
public:
    enum class pool_type {
        max,
        avg,
    };

    pool_layer(
        io_shape in_shape = {1, 0, 0},
        int kernel_size = 3,
        int stride = 3,
        pool_type p_type = pool_type::max,
        bool training = true);

    std::vector<std::vector<Eigen::MatrixXd>> forward(
        const std::vector<std::vector<Eigen::MatrixXd>>& input) override;

    std::vector<std::vector<Eigen::MatrixXd>> backward(
        const std::vector<std::vector<Eigen::MatrixXd>>& grad_output,
        double learning_rate) override;

    void save(std::ostream& os) const override;
    void load(std::istream& is) override;

    pool_type p_type() const { return p_type_; }
    int stride() const { return stride_; }
    int kernel_size() const { return kernel_size_; }

private:
    pool_type p_type_;
    int stride_;
    int kernel_size_;
    std::vector<std::vector<Eigen::MatrixXi>> max_indices_;
};

}  // namespace core