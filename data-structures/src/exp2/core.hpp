#include <memory>
#include <unordered_map>
#include <vector>
#include "utils.hpp"

namespace e2 {
struct huffman_node {
    byte data;
    uint64_t freq;
    std::shared_ptr<huffman_node> lc, rc;

    huffman_node(byte d, uint64_t f) : data(d), freq(f), lc(nullptr), rc(nullptr) {}
    huffman_node(uint64_t f, std::shared_ptr<huffman_node> l, std::shared_ptr<huffman_node> r) : data(0), freq(f), lc(l), rc(r) {}
    bool is_leaf() const { return lc == nullptr && rc == nullptr; }
    uint64_t height() const {
        if (this->is_leaf()) {
            return 0;
        }
        return std::max(lc->height(), rc->height()) + 1;
    }
    struct ptr_compare {
        bool operator()(std::shared_ptr<huffman_node> a, std::shared_ptr<huffman_node> b) {
            if (a->freq != b->freq) return a->freq > b->freq;
            if (a->height() != b->height()) return a->height() > b->height();
            return a->data > b->data;
        }
    };
};

std::shared_ptr<huffman_node> build_tree(const std::unordered_map<byte, uint64_t>& freq_table);
std::unordered_map<byte, uint64_t> build_freq_table(const std::vector<byte>& vec_data);

void generate_codes(std::shared_ptr<huffman_node> node, std::vector<bool>& current_code, std::unordered_map<byte, std::vector<bool>>& codes);
void serialize_data(byte data, std::vector<bool>& buffer);
void serialize_tree(std::shared_ptr<huffman_node> node, std::vector<bool>& buffer);
byte deserialize_data(const std::vector<bool>& buffer, uint64_t& pos);
std::shared_ptr<huffman_node> deserialize_tree(const std::vector<bool>& buffer, uint64_t& pos);

std::vector<bool> encode(const std::vector<byte>& data, std::unordered_map<byte, std::vector<bool>>& codes);
std::pair<std::vector<bool>, std::string> encode_winfo(const std::vector<byte>& data, std::unordered_map<byte, std::vector<bool>>& codes);
std::vector<byte> decode(std::shared_ptr<huffman_node> root, const std::vector<bool>& encoded);
}  // namespace e2