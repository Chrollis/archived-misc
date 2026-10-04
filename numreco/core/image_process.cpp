#include "image_process.hpp"
#include <QCryptographicHash>
#undef min
#undef max

namespace chr {
namespace image_process {

cv::Mat eigen_matrix_to_cv_mat(const Eigen::MatrixXd& matrix) {
    int rows = static_cast<int>(matrix.rows()), cols = static_cast<int>(matrix.cols());
    cv::Mat image(rows, cols, CV_8UC1);
    for (int m = 0; m < rows; m++) {
        unsigned char* ptr = image.ptr<unsigned char>(m);
        for (int n = 0; n < cols; n++) {
            ptr[n] = static_cast<unsigned char>(matrix(m, n) * 255);
        }
    }
    return image;
}
Eigen::MatrixXd cv_mat_to_eigen_matrix(const cv::Mat& mat) {
    Eigen::MatrixXd matrix(mat.rows, mat.cols);
    for (int i = 0; i < mat.rows; i++) {
        for (int j = 0; j < mat.cols; j++) {
            matrix(i, j) = mat.at<unsigned char>(i, j) ? 1.0 : 0.0;
        }
    }
    return matrix;
}

Eigen::MatrixXd process_digit(const cv::Mat& digit_mat) {
    cv::Mat copy = digit_mat.clone();
    apply_padding(copy);
    cv::Mat resized;
    cv::resize(copy, resized, cv::Size(28, 28), 0, 0, cv::INTER_AREA);
    return cv_mat_to_eigen_matrix(resized);
}

void apply_padding(cv::Mat& img) {
    int top, bottom, left, right;

    if (img.rows > img.cols) {
        left = (img.rows - img.cols) / 2;
        right = img.rows - img.cols - left;
        top = bottom = 0;
    } else {
        top = (img.cols - img.rows) / 2;
        bottom = img.cols - img.rows - top;
        left = right = 0;
    }

    cv::copyMakeBorder(img, img, top, bottom, left, right, cv::BORDER_CONSTANT, cv::Scalar(0));

    int border_width = img.rows / 5;
    cv::copyMakeBorder(img, img, border_width, border_width, border_width, border_width, cv::BORDER_CONSTANT, cv::Scalar(0));
}

bool is_valid_digit_region(const cv::Rect& rect, const cv::Size& image_size) {
    int min_size = static_cast<int>(std::min(image_size.width, image_size.height) * 0.1);
    min_size = std::min(min_size, 28);
    return (rect.width > min_size || rect.height > min_size);
}

std::vector<digit_block> process_image(const cv::Mat& digits_mat) {
    std::vector<digit_block> digits;
    cv::Mat processed = binarize_image(digits_mat);
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;

    cv::findContours(processed, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
    for (size_t i = 0; i < contours.size(); i++) {
        cv::Rect bounding_rect = cv::boundingRect(contours[i]);
        if (is_valid_digit_region(bounding_rect, digits_mat.size())) {
            cv::Mat digit_mat = processed(bounding_rect);
            Eigen::MatrixXd digit = process_digit(digit_mat);
            if (digit.size() > 0) {
                digits.push_back({bounding_rect, digit});
            }
        }
    }
    if (digits.size() <= 1) return digits;

    std::vector<int> heights;
    for (const auto& digit : digits) {
        heights.push_back(digit.rect.height);
    }
    std::sort(heights.begin(), heights.end());
    int median_height = heights[heights.size() / 2];

    std::sort(digits.begin(), digits.end(), [](const digit_block& a, const digit_block& b) { return a.rect.y < b.rect.y; });

    std::vector<std::vector<digit_block>> rows;
    for (const auto& digit : digits) {
        if (rows.empty()) {
            rows.push_back({digit});
        } else {
            auto& curr_row = rows.back();
            int curr_row_y = curr_row[0].rect.y;
            if (std::abs(digit.rect.y - curr_row_y) <= median_height * 0.8) {
                curr_row.push_back(digit);
            } else {
                rows.push_back({digit});
            }
        }
    }

    for (auto& row : rows) {
        std::sort(row.begin(), row.end(), [](const digit_block& a, const digit_block& b) { return a.rect.x < b.rect.x; });
    }
    digits.clear();
    for (const auto& row : rows) {
        for (const auto& digit : row) {
            digits.push_back(digit);
        }
    }
    return digits;
}

cv::Mat labelize_image(const cv::Mat& src, const std::vector<digit_block>& blocks) {
    cv::Mat dst = src;
    for (size_t i = 0; i < blocks.size(); i++) {
        int x = blocks[i].rect.x;
        int y = blocks[i].rect.y;
        int w = blocks[i].rect.width;
        int h = blocks[i].rect.height;
        dst.row(y).colRange(x, x + w - 1).setTo(cv::Scalar(0, 255, 0));
        dst.row(y + h - 1).colRange(x, x + w - 1).setTo(cv::Scalar(0, 255, 0));
        dst.col(x).rowRange(y, y + h - 1).setTo(cv::Scalar(0, 255, 0));
        dst.col(x + w - 1).rowRange(y, y + h - 1).setTo(cv::Scalar(0, 255, 0));
    }
    return dst;
}

cv::Mat binarize_image(const cv::Mat& src_img) {
    cv::Mat gray;

    if (src_img.channels() == 3) {
        cv::cvtColor(src_img, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = src_img.clone();
    }
    cv::Mat binary;
    cv::threshold(gray, binary, 127, 255, cv::THRESH_BINARY_INV);
    return binary;
}
cv::Mat qimage_to_cv_mat(const QImage& qimage) {
    cv::Mat mat;
    switch (qimage.format()) {
        case QImage::Format_RGB32:
        case QImage::Format_ARGB32:
        case QImage::Format_ARGB32_Premultiplied: {
            mat = cv::Mat(qimage.height(), qimage.width(), CV_8UC4, const_cast<uchar*>(qimage.bits()), static_cast<size_t>(qimage.bytesPerLine()));
            cv::cvtColor(mat, mat, cv::COLOR_BGRA2BGR);
            break;
        }
        case QImage::Format_RGB888: {
            mat = cv::Mat(qimage.height(), qimage.width(), CV_8UC3, const_cast<uchar*>(qimage.bits()), static_cast<size_t>(qimage.bytesPerLine()));
            cv::cvtColor(mat, mat, cv::COLOR_RGB2BGR);
            break;
        }
        case QImage::Format_Grayscale8: {
            mat = cv::Mat(qimage.height(), qimage.width(), CV_8UC1, const_cast<uchar*>(qimage.bits()), static_cast<size_t>(qimage.bytesPerLine()));
            break;
        }
        default: {
            QImage converted = qimage.convertToFormat(QImage::Format_RGB888);
            mat = cv::Mat(converted.height(), converted.width(), CV_8UC3, const_cast<uchar*>(converted.bits()), static_cast<size_t>(converted.bytesPerLine()));
            cv::cvtColor(mat, mat, cv::COLOR_RGB2BGR);
            break;
        }
    }
    return mat.clone();
}
QImage cv_mat_to_qimage(const cv::Mat& mat) {
    switch (mat.type()) {
        case CV_8UC1: {
            QImage image(mat.data, mat.cols, mat.rows, static_cast<int>(mat.step), QImage::Format_Grayscale8);
            return image.copy();
        }
        case CV_8UC3: {
            QImage image(mat.data, mat.cols, mat.rows, static_cast<int>(mat.step), QImage::Format_RGB888);
            return image.rgbSwapped().copy();
        }
        case CV_8UC4: {
            QImage image(mat.data, mat.cols, mat.rows, static_cast<int>(mat.step), QImage::Format_ARGB32);
            return image.copy();
        }
        default: {
            cv::Mat converted;
            mat.convertTo(converted, CV_8UC3);
            QImage image(converted.data, converted.cols, converted.rows, static_cast<int>(converted.step), QImage::Format_RGB888);
            return image.rgbSwapped().copy();
        }
    }
}
digit_block::digit_block(const cv::Rect& rect, const Eigen::MatrixXd data) {
    this->rect = rect;
    this->data = data;
    qint64 timestamp = QDateTime::currentMSecsSinceEpoch();
    QCryptographicHash qhash(QCryptographicHash::Sha256);
    qhash.addData(reinterpret_cast<const char*>(&timestamp));
    QByteArray bits;
    unsigned char bit = 0;
    int counter = 0;
    for (int i = 0; i < data.rows(); i++) {
        for (int j = 0; j < data.cols(); j++) {
            if (data(i, j) > 0.5) {
                bit |= (1 << (7 - counter));
            }
            if (++counter == 8) {
                bits.append(bit);
                bit = 0;
                counter = 0;
            }
        }
    }
    if (counter > 0) {
        bits.append(bit);
    }
    qhash.addData(bits);
    hash = qhash.result().toHex();
}
}  // namespace image_process
}  // namespace chr
