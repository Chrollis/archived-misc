#ifndef CONVOLVE_LAYER_HPP
#define CONVOLVE_LAYER_HPP

#include <Eigen/Dense>
#include "activation_function.hpp"
#include "filter.hpp"

namespace chr {
class convolve_layer {
private:
    size_t in_channels_;
    size_t kernel_size_;
    size_t out_channels_;
    size_t padding_;
    size_t stride_;
    activation_function afunc_;
    std::vector<filter> filters_;
    std::vector<Eigen::MatrixXd> input_;
    std::vector<Eigen::MatrixXd> feature_map_;
    std::vector<Eigen::MatrixXd> convolve_outcome_;

public:
    convolve_layer(size_t in_channels, size_t kernel_size, size_t out_channels, size_t padding = 0, size_t stride = 1, activation_function_type activate_type = activation_function_type::relu);
    std::vector<Eigen::MatrixXd> forward(const std::vector<Eigen::MatrixXd>& input);
    std::vector<Eigen::MatrixXd> backward(const std::vector<Eigen::MatrixXd>& gradient, double learning_rate, bool is_last_conv = false);
    void weights_update(double learning_rate, const std::vector<Eigen::MatrixXd>& gradient);
    void save(std::ostream& file) const;
    void load(std::istream& file);

private:
    std::vector<Eigen::MatrixXd> padding(const std::vector<Eigen::MatrixXd>& input, size_t circle_num, double fill_num = 0.0) const;
    Eigen::MatrixXd convolve(const Eigen::MatrixXd& input, const Eigen::MatrixXd& kernel, size_t stride) const;
    std::vector<Eigen::MatrixXd> apply_activation_derivative(const std::vector<Eigen::MatrixXd>& gradient);
    std::vector<Eigen::MatrixXd> remove_padding(const std::vector<Eigen::MatrixXd>& input, size_t padding);
};
}  // namespace chr

#endif
