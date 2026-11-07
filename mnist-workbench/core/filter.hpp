#ifndef FILTER_HPP
#define FILTER_HPP

#include <Eigen/Dense>
#include <filesystem>

namespace chr {

class filter {
private:
    size_t core_size_;
    size_t channels_;

public:
    double bias;
    std::vector<Eigen::MatrixXd> kernels;

public:
    filter(size_t channels, size_t core_size);
    void initialize_gausz(double stddev);
    void initialize_xavier(size_t input_size);
    void initialize_He(size_t input_size);
};
}  // namespace chr

#endif
