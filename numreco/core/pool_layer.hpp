#ifndef POOL_LAYER_HPP
#define POOL_LAYER_HPP

#include <Eigen/Dense>

namespace chr {
enum class pooling_type { max, average };

class pool_layer {
private:
    size_t core_size_;
    size_t stride_;
    pooling_type type_;
    std::vector<Eigen::MatrixXd> input_;
    std::vector<Eigen::MatrixXd> feature_map_;
    std::vector<Eigen::MatrixXd> record_;

public:
    pool_layer(size_t core_size, size_t stride, pooling_type type = pooling_type::max);
    std::vector<Eigen::MatrixXd> forward(const std::vector<Eigen::MatrixXd>& input);
    std::vector<Eigen::MatrixXd> backward(const std::vector<Eigen::MatrixXd>& gradient);

private:
    void max_pooling(const Eigen::MatrixXd& input, Eigen::MatrixXd& output, Eigen::MatrixXd& record) const;
    void average_pooling(const Eigen::MatrixXd& input, Eigen::MatrixXd& output) const;
    Eigen::MatrixXd max_backward(const Eigen::MatrixXd& gradient, const Eigen::MatrixXd& record) const;
    Eigen::MatrixXd average_backward(const Eigen::MatrixXd& gradient);
};

}  // namespace chr

#endif
