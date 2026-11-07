#ifndef LE_NET5_HPP
#define LE_NET5_HPP

#include "cnn_base.h"
#include "convolve_layer.hpp"
#include "full_connect_layer.hpp"
#include "pool_layer.hpp"

namespace chr {
class le_net5 : public cnn_base {
private:
    convolve_layer conv1_;
    pool_layer pool1_;
    convolve_layer conv2_;
    pool_layer pool2_;
    full_connect_layer fc1_;
    full_connect_layer fc2_;
    full_connect_layer fc3_;

public:
    le_net5();
    Eigen::VectorXd forward(const std::vector<Eigen::MatrixXd>& input) override;
    std::vector<Eigen::MatrixXd> backward(size_t label, double learning_rate) override;
    void save(const std::filesystem::path& path) override;
    void load(const std::filesystem::path& path) override;
    std::string model_type() const override { return "LeNet-5"; }
};
}  // namespace chr

#endif
