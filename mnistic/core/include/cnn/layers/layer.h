#pragma once

#include <Eigen/Dense>
#include <istream>
#include <ostream>
#include <vector>

namespace core {

class layer {
public:
    struct io_shape {
        int c;
        int w;
        int h;
    };

    virtual ~layer() = default;

    virtual std::vector<std::vector<Eigen::MatrixXd>> forward(
        const std::vector<std::vector<Eigen::MatrixXd>>& input) = 0;

    virtual std::vector<std::vector<Eigen::MatrixXd>> backward(
        const std::vector<std::vector<Eigen::MatrixXd>>& grad_output,
        double learning_rate) = 0;

    virtual void save(std::ostream& os) const = 0;
    virtual void load(std::istream& is) = 0;

    void train() { training_ = true; }
    void eval() { training_ = false; }
    bool is_training() const { return training_; }

    io_shape input_shape() const { return input_shape_; }
    io_shape output_shape() const { return output_shape_; }

    enum class layer_type {
        base,
        convolve,
        pool,
        flatten,
        dense,
        dropout,
        batchnorm,
        activate,
        softmax,
    };

    layer_type type() const { return type_; }

    static void save_type(std::ostream& os, layer_type type);
    static layer_type load_type(std::istream& is);

protected:
    layer(
        io_shape in_shape = {1, 0, 0},
        io_shape out_shape = {1, 0, 0},
        layer_type type = layer_type::base,
        bool training = true);

    void save_header(std::ostream& os) const;
    void load_header(std::istream& is);

    io_shape input_shape_;
    io_shape output_shape_;
    layer_type type_;
    bool training_;
};

}  // namespace core