#pragma once

#include <cnn/layers/layer.h>
#include <random>
#include <vector>

namespace core {

class conv_layer : public layer {
public:
    enum class init_type {
        gaussian,
        xavier,
        he,
        load,
    };

    conv_layer(
        io_shape in_shape = {1, 0, 0},
        int out_channels = 1,
        int kernel_radius = 1,
        init_type init = init_type::he,
        int stride = 1,
        int padding = 0,
        bool training = true);

    std::vector<std::vector<Eigen::MatrixXd>> forward(
        const std::vector<std::vector<Eigen::MatrixXd>>& input) override;

    std::vector<std::vector<Eigen::MatrixXd>> backward(
        const std::vector<std::vector<Eigen::MatrixXd>>& grad_output,
        double learning_rate) override;

    void save(std::ostream& os) const override;
    void load(std::istream& is) override;

    int kernel_size() const { return kernel_size_; }
    int stride() const { return stride_; }
    int padding() const { return padding_; }

private:
    struct conv_kernel {
        std::vector<Eigen::MatrixXd> weights;
        double bias;

        conv_kernel(
            init_type type = init_type::load,
            int in_channels = 0,
            int kernel_size = 1);
    };

    int kernel_size_;
    int stride_;
    int padding_;

    std::vector<conv_kernel> kernels_;

    std::vector<std::vector<Eigen::MatrixXd>> cached_input_;

    static std::mt19937& rng();

    static std::vector<Eigen::MatrixXd> convolve_general(
        const std::vector<Eigen::MatrixXd>& padded_input,
        const std::vector<conv_kernel>& kernels,
        int stride,
        int out_h,
        int out_w);
};

}  // namespace core