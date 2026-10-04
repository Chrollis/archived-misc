#ifndef ACTIVATION_FUNCTION_HPP
#define ACTIVATION_FUNCTION_HPP

#include <cmath>
#include <stdexcept>

namespace chr {

enum class activation_function_type { relu, lrelu, sigmoid, tanh, softmax };

class activation_function {
public:
    activation_function(activation_function_type type = activation_function_type::relu) : type_(type) {}

    double operator()(double x) const {
        switch (type_) {
            case activation_function_type::relu:

                return x > 0.0 ? x : 0.0;
            case activation_function_type::lrelu:

                return x > 0.0 ? x : 0.01 * x;
            case activation_function_type::sigmoid:
                return 1.0 / (1.0 + std::exp(-x));
            case activation_function_type::tanh:
                return std::tanh(x);
            case activation_function_type::softmax:

                return x;
            default:
                throw std::runtime_error("unknown activation function type");
        }
    }

    double operator[](double x) const {
        switch (type_) {
            case activation_function_type::relu:

                return x > 0.0 ? 1.0 : 0.0;
            case activation_function_type::lrelu:

                return x > 0.0 ? 1.0 : 0.01;
            case activation_function_type::sigmoid: {
                double s = operator()(x);
                return s * (1.0 - s);
            }
            case activation_function_type::tanh: {
                double t = std::tanh(x);
                return 1.0 - t * t;
            }
            case activation_function_type::softmax:

                return 1.0;
            default:
                throw std::runtime_error("unknown activation function type");
        }
    }

private:
    activation_function_type type_;
};

}  // namespace chr

#endif
