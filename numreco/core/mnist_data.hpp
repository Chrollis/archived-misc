#ifndef MNIST_DATA_HPP
#define MNIST_DATA_HPP

#include <filesystem>
#include "image_process.hpp"

namespace chr {

class mnist_data {
private:
    Eigen::MatrixXd image_;
    size_t label_;

public:
    mnist_data(const Eigen::MatrixXd& image, size_t label);
    mnist_data(Eigen::MatrixXd&& image, size_t label);
    const Eigen::MatrixXd& image() const { return image_; }
    cv::Mat cv_image() const { return image_process::eigen_matrix_to_cv_mat(image_); }
    size_t label() const { return label_; }
    bool is_legal() const;

public:
    static unsigned swap_endian(unsigned val);
    static unsigned check_mnist_file(std::ifstream& mnist_images, std::ifstream& mnist_labels);

public:
    static std::vector<mnist_data> obtain_data(const std::filesystem::path& mnist_image_path, const std::filesystem::path& mnist_label_path, size_t offset = 0, size_t size = 60000);
    static void write_data(const std::filesystem::path& image_path, const std::filesystem::path& label_path, const std::vector<mnist_data>& datas);
};
}  // namespace chr

#endif
