#pragma once

#include <cnn/layers/layer.h>

namespace core {

class activate {
public:
    enum class func_type {
        sigmoid,
        tanh,
        relu,
        lrelu,
        linear,
    };

    explicit activate(func_type t) : type_(t) {}

    double forward(double x) const;
    double backward(double x) const;

    Eigen::MatrixXd forward(const Eigen::MatrixXd& x) const;
    Eigen::MatrixXd backward(const Eigen::MatrixXd& x) const;

    void set_type(func_type t) { type_ = t; }
    func_type type() const { return type_; }

private:
    func_type type_;
};

class activate_layer : public layer {
public:
    explicit activate_layer(
        io_shape in_shape = {1, 0, 0},
        bool training = true,
        activate::func_type act_type = activate::func_type::relu);

    std::vector<std::vector<Eigen::MatrixXd>> forward(
        const std::vector<std::vector<Eigen::MatrixXd>>& input) override;

    std::vector<std::vector<Eigen::MatrixXd>> backward(
        const std::vector<std::vector<Eigen::MatrixXd>>& grad_output,
        double learning_rate) override;

    void save(std::ostream& os) const override;
    void load(std::istream& is) override;

    activate::func_type act_type() const { return act_.type(); }

private:
    activate act_;
    std::vector<std::vector<Eigen::MatrixXd>> cached_input_;
};

}  // namespace core