#include "core.hpp"
#include <queue>

namespace e2 {
std::shared_ptr<huffman_node> build_tree(const std::unordered_map<byte, uint64_t>& freq_table) {
    std::priority_queue<std::shared_ptr<huffman_node>, std::vector<std::shared_ptr<huffman_node>>, huffman_node::ptr_compare> heap;
    for (auto& [d, f] : freq_table) {
        heap.push(std::make_shared<huffman_node>(d, f));
    }
    if (heap.empty()) {
        return nullptr;
    }
    if (heap.size() == 1) {
        return std::make_shared<huffman_node>(heap.top()->freq, heap.top(), nullptr);
    }

    while (heap.size() > 1) {
        auto l = heap.top();
        heap.pop();
        auto r = heap.top();
        heap.pop();
        heap.push(std::make_shared<huffman_node>(l->freq + r->freq, l, r));
    }
    return heap.top();
}
std::unordered_map<byte, uint64_t> build_freq_table(const std::vector<byte>& vec_data) {
    std::unordered_map<byte, uint64_t> freq_table;
    for (byte data : vec_data) {
        freq_table[data]++;
    }
    return freq_table;
}

void generate_codes(std::shared_ptr<huffman_node> node, std::vector<bool>& current_code, std::unordered_map<byte, std::vector<bool>>& codes) {
    if (node == nullptr) return;
    if (node->is_leaf()) {
        codes[node->data] = current_code;
        return;
    }
    if (node->lc) {
        current_code.push_back(0);
        generate_codes(node->lc, current_code, codes);
        current_code.pop_back();
    }
    if (node->rc) {
        current_code.push_back(1);
        generate_codes(node->rc, current_code, codes);
        current_code.pop_back();
    }
}
void serialize_data(byte data, std::vector<bool>& buffer) {
    for (int i = 7; i >= 0; --i) {
        buffer.push_back((data >> i) & 1);
    }
}
void serialize_tree(std::shared_ptr<huffman_node> node, std::vector<bool>& buffer) {
    if (node == nullptr) return;
    if (node->is_leaf()) {
        buffer.push_back(1);
        serialize_data(node->data, buffer);
    } else {
        buffer.push_back(0);
        serialize_tree(node->lc, buffer);
        serialize_tree(node->rc, buffer);
    }
}
byte deserialize_data(const std::vector<bool>& buffer, uint64_t& pos) {
    byte data = 0;
    for (int i = 7; i >= 0; --i) {
        data |= (buffer[pos++] << i);
    }
    return data;
}
std::shared_ptr<huffman_node> deserialize_tree(const std::vector<bool>& buffer, uint64_t& pos) {
    if (pos >= buffer.size()) return nullptr;
    if (buffer[pos++]) {
        byte data = deserialize_data(buffer, pos);
        return std::make_shared<huffman_node>(data, 0);
    } else {
        auto l = deserialize_tree(buffer, pos);
        auto r = deserialize_tree(buffer, pos);
        return std::make_shared<huffman_node>(0, l, r);
    }
}

std::vector<bool> encode(const std::vector<byte>& data, std::unordered_map<byte, std::vector<bool>>& codes) {
    std::vector<bool> result;
    for (byte d : data) {
        auto seg = codes[d];
        result.insert(result.end(), seg.begin(), seg.end());
    }
    return result;
}
std::pair<std::vector<bool>, std::string> encode_winfo(const std::vector<byte>& data, std::unordered_map<byte, std::vector<bool>>& codes) {
    std::vector<bool> result = encode(data, codes);
    uint64_t original = data.size() * 8;
    uint64_t encoded = result.size();
    double comp_ratio = (1.0 - static_cast<double>(encoded) / original) * 100;
    return {result, std::format("Ori: {}; Enc: {}; Comp: {:.2f}", original, encoded, comp_ratio)};
}
std::vector<byte> decode(std::shared_ptr<huffman_node> root, const std::vector<bool>& encoded) {
    std::vector<byte> result;
    if (root == nullptr || encoded.empty()) return result;
    auto it = std::back_inserter(result);
    for (uint64_t pos = 0; pos < encoded.size();) {
        auto node = root;
        for (; !node->is_leaf();) {
            if (encoded[pos++]) {
                node = node->rc;
            } else {
                node = node->lc;
            }
        }
        *it++ = node->data;
    }
    return result;
}

}  // namespace e2