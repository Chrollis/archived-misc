#ifndef FULL_CONNECT_LAYER_HPP
#define FULL_CONNECT_LAYER_HPP

#include <Eigen/Dense>
#include "activation_function.hpp"

namespace chr {
class full_connect_layer {
private:
    size_t in_size_;
    size_t out_size_;
    activation_function afunc_;
    Eigen::MatrixXd weights_;
    Eigen::VectorXd biases_;
    Eigen::VectorXd input_;
    Eigen::VectorXd gradient_;
    Eigen::VectorXd feature_vector_;
    Eigen::VectorXd linear_outcome_;

public:
    full_connect_layer(size_t in_size, size_t out_size, activation_function_type activate_type = activation_function_type::relu);
    Eigen::VectorXd forward(const Eigen::VectorXd& input);
    Eigen::VectorXd backward(const Eigen::VectorXd& gradient, double learning_rate, bool is_output_layer = false, size_t label = 0);
    void weights_update(double learning_rate);
    void save(std::ostream& file) const;
    void load(std::istream& file);

private:
    void initialize_weights();
};

}  // namespace chr

#endif
