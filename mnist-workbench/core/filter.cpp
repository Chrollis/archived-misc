#include "filter.hpp"
#include <random>

namespace chr {
filter::filter(size_t channels, size_t core_size) : core_size_(core_size), channels_(channels) {
    initialize_gausz(0.01);
}

void filter::initialize_gausz(double stddev) {
    kernels.resize(channels_);
    std::random_device rd;
    std::default_random_engine generator(rd());
    std::normal_distribution<double> distributor(0.0, stddev);
    bias = distributor(generator);
    for (size_t i = 0; i < channels_; i++) {
        kernels[i] = Eigen::MatrixXd::Zero(core_size_, core_size_);
        for (size_t m = 0; m < core_size_; m++) {
            for (size_t n = 0; n < core_size_; n++) {
                kernels[i](m, n) = distributor(generator);
            }
        }
    }
}

void filter::initialize_xavier(size_t input_size) {
    kernels.resize(channels_);
    std::random_device rd;
    std::default_random_engine generator(rd());

    double limit = sqrt(6.0 / (input_size + channels_ * core_size_ * core_size_));
    std::uniform_real_distribution<double> distributor(-limit, limit);
    bias = 0;
    for (size_t i = 0; i < channels_; i++) {
        kernels[i] = Eigen::MatrixXd::Zero(core_size_, core_size_);
        for (size_t m = 0; m < core_size_; m++) {
            for (size_t n = 0; n < core_size_; n++) {
                kernels[i](m, n) = distributor(generator);
            }
        }
    }
}

void filter::initialize_He(size_t input_size) {
    kernels.resize(channels_);
    std::random_device rd;
    std::default_random_engine generator(rd());

    double stddev = sqrt(2.0 / input_size);
    std::normal_distribution<double> distributor(0.0, stddev);
    bias = 0;
    for (size_t i = 0; i < channels_; i++) {
        kernels[i] = Eigen::MatrixXd::Zero(core_size_, core_size_);
        for (size_t m = 0; m < core_size_; m++) {
            for (size_t n = 0; n < core_size_; n++) {
                kernels[i](m, n) = distributor(generator);
            }
        }
    }
}
}  // namespace chr
