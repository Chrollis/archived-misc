#pragma once

#include <cnn/layers/layer.h>
#include <istream>
#include <memory>
#include <ostream>
#include <vector>

namespace core {

class sequential {
public:
    sequential() = default;
    ~sequential() = default;

    sequential(const sequential&) = delete;
    sequential& operator=(const sequential&) = delete;
    sequential(sequential&&) = default;
    sequential& operator=(sequential&&) = default;

    void add_layer(std::unique_ptr<layer> l);

    size_t size() const { return layers_.size(); }
    bool empty() const { return layers_.empty(); }

    std::vector<std::vector<Eigen::MatrixXd>> forward(
        const std::vector<std::vector<Eigen::MatrixXd>>& input);

    std::vector<std::vector<Eigen::MatrixXd>> backward(
        const std::vector<std::vector<Eigen::MatrixXd>>& grad_output,
        double learning_rate);

    void train();
    void eval();

    void save(std::ostream& os) const;
    void load(std::istream& is);

    layer::io_shape input_shape() const;
    layer::io_shape output_shape() const;
    const std::vector<std::unique_ptr<layer>>& layers() const {
        return layers_;
    }

private:
    std::vector<std::unique_ptr<layer>> layers_;

    static std::unique_ptr<layer> make_layer(layer::layer_type type);
};

}  // namespace core