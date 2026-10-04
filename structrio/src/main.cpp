#include <cstdint>
#include <format>
#include <fstream>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "exp1/core.hpp"
#include "exp2/core.hpp"
#include "exp3/core.hpp"
#include "ui.hpp"

void evaluate_once() {
    std::string line = ui::prompt("expression (empty cancels)");
    if (line.empty()) {
        ui::info("cancelled");
        return;
    }
    e1::tokenizer tk;
    if (!tk.validate(line)) {
        ui::error("invalid expression");
        for (const auto& [tok, desc] : tk.errs) ui::error("  " + tok + ": " + desc);
        return;
    }
    try {
        e1::expression e(line);
        std::cout << ui::cyan << "  infix  : " << ui::reset << e.infix_expr() << '\n' << ui::cyan << "  postfix: " << ui::reset << e.postfix_expr() << '\n';
        ui::success(std::format("= {}", e.eval_postfix()));
    } catch (const std::exception& ex) {
        ui::error(ex.what());
    }
}

void run_e1() {
    ui::print_title("e1 - expression evaluator");
    for (;;) {
        int c = ui::select(
            "action", {
                          "evaluate an expression",
                          "back to main menu",
                      });
        if (c == 1 || c < 0) return;
        evaluate_once();
    }
}

using byte = unsigned char;

std::vector<byte> pack_bits(const std::vector<bool>& bits) {
    std::vector<byte> out((bits.size() + 7) / 8, 0);
    for (std::size_t i = 0; i < bits.size(); ++i)
        if (bits[i]) out[i / 8] |= byte(1u << (7 - (i % 8)));
    return out;
}

std::vector<bool> unpack_bits(const std::vector<byte>& v, std::size_t offset_bytes, std::size_t nbits) {
    std::vector<bool> out(nbits);
    for (std::size_t i = 0; i < nbits; ++i) out[i] = (v[offset_bytes + i / 8] >> (7 - (i % 8))) & 1u;
    return out;
}

void put_u64(std::vector<byte>& v, uint64_t x) {
    for (int i = 7; i >= 0; --i) v.push_back(byte((x >> (i * 8)) & 0xFF));
}

uint64_t get_u64(const std::vector<byte>& v, std::size_t& pos) {
    uint64_t x = 0;
    for (int i = 0; i < 8; ++i) x = (x << 8) | v[pos++];
    return x;
}

std::vector<byte> read_file(const std::filesystem::path& p) {
    std::ifstream f(p, std::ios::binary);
    if (!f) throw std::runtime_error(std::format("cannot open {}", p.string()));
    return {std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>()};
}

void write_file(const std::filesystem::path& p, const std::vector<byte>& v) {
    std::ofstream f(p, std::ios::binary);
    if (!f) throw std::runtime_error(std::format("cannot write {}", p.string()));
    f.write(reinterpret_cast<const char*>(v.data()), std::streamsize(v.size()));
}

struct Encoded {
    std::shared_ptr<e2::huffman_node> root;
    std::vector<bool> bits;
    std::string report;
};

Encoded encode_bytes(const std::vector<byte>& data) {
    auto freq = e2::build_freq_table(data);
    auto root = e2::build_tree(freq);
    std::unordered_map<byte, std::vector<bool>> codes;
    std::vector<bool> cur;
    e2::generate_codes(root, cur, codes);
    auto [bits, info] = e2::encode_winfo(data, codes);
    return {root, std::move(bits), std::move(info)};
}

void do_encode_string() {
    std::string s = ui::prompt("text to encode (empty cancels)");
    if (s.empty()) {
        ui::info("cancelled");
        return;
    }
    std::vector<byte> data(s.begin(), s.end());

    auto enc = encode_bytes(data);
    ui::success(enc.report);
    auto back = e2::decode(enc.root, enc.bits);
    if (back == data)
        ui::success("roundtrip ok");
    else
        ui::error("roundtrip FAILED");
}

void do_encode_file() {
    std::string in = ui::prompt("input path (empty cancels)");
    if (in.empty()) {
        ui::info("cancelled");
        return;
    }
    std::string out = ui::prompt("output path", in + ".huff");
    if (std::filesystem::exists(out) && !ui::confirm("overwrite " + out + "?", false)) return;

    try {
        auto data = read_file(in);
        auto enc = encode_bytes(data);

        std::vector<bool> tree_bits;
        e2::serialize_tree(enc.root, tree_bits);

        std::vector<byte> outbuf;
        put_u64(outbuf, tree_bits.size());
        auto tb = pack_bits(tree_bits);
        outbuf.insert(outbuf.end(), tb.begin(), tb.end());
        put_u64(outbuf, enc.bits.size());
        auto pb = pack_bits(enc.bits);
        outbuf.insert(outbuf.end(), pb.begin(), pb.end());

        write_file(out, outbuf);
        ui::success(enc.report);
        ui::success(std::format("wrote {} ({} bytes)", out, outbuf.size()));
    } catch (const std::exception& ex) {
        ui::error(ex.what());
    }
}

void do_decode_file() {
    std::string in = ui::prompt("input .huff path (empty cancels)");
    if (in.empty()) {
        ui::info("cancelled");
        return;
    }
    std::string out = ui::prompt("output path", in + ".out");
    if (std::filesystem::exists(out) && !ui::confirm("overwrite " + out + "?", false)) return;

    try {
        auto buf = read_file(in);
        std::size_t pos = 0;
        uint64_t tree_nbits = get_u64(buf, pos);
        std::size_t tree_bytes = (tree_nbits + 7) / 8;
        std::size_t tree_off = pos;
        pos += tree_bytes;
        uint64_t pay_nbits = get_u64(buf, pos);
        std::size_t pay_off = pos;

        auto tree_bits = unpack_bits(buf, tree_off, tree_nbits);
        std::size_t tpos = 0;
        auto root = e2::deserialize_tree(tree_bits, tpos);

        auto pay_bits = unpack_bits(buf, pay_off, pay_nbits);
        auto data = e2::decode(root, pay_bits);

        write_file(out, data);
        ui::success(std::format("wrote {} ({} bytes)", out, data.size()));
    } catch (const std::exception& ex) {
        ui::error(ex.what());
    }
}

void run_e2() {
    ui::print_title("e2 - huffman coder");
    for (;;) {
        int c = ui::select(
            "action", {
                          "encode a string (in-memory)",
                          "encode a file",
                          "decode a file",
                          "back to main menu",
                      });
        if (c < 0 || c == 3) return;
        switch (c) {
            case 0:
                do_encode_string();
                break;
            case 1:
                do_encode_file();
                break;
            case 2:
                do_decode_file();
                break;
        }
    }
}

std::optional<uint64_t> pick_place(const e3::plat& world, std::string_view q) {
    auto kw = ui::prompt(q);
    if (kw.empty()) return std::nullopt;
    auto hits = world.fuzzy_find_places(kw);
    if (hits.empty()) {
        ui::warning("no place matched");
        return std::nullopt;
    }
    std::vector<std::string> disp;
    disp.reserve(hits.size());
    for (auto& [id, name] : hits) disp.push_back(std::format("[{:016X}] {}", id, name));
    int c = ui::select("pick a place", disp);
    if (c < 0) return std::nullopt;
    return hits[c].first;
}

std::optional<uint32_t> pick_city(const e3::plat& world, std::string_view q) {
    auto kw = ui::prompt(q);
    if (kw.empty()) return std::nullopt;
    auto hits = world.fuzzy_find_cities(kw);
    if (hits.empty()) {
        ui::warning("no city matched");
        return std::nullopt;
    }
    std::vector<std::string> disp;
    disp.reserve(hits.size());
    for (auto& [id, name] : hits) disp.push_back(std::format("[{}] {}", id, name));
    int c = ui::select("pick a city", disp);
    if (c < 0) return std::nullopt;
    return hits[c].first;
}

void do_load(e3::plat& world) {
    std::string p = ui::prompt("map json path (empty cancels)");
    if (p.empty()) {
        ui::info("cancelled");
        return;
    }
    try {
        world.load(p);
        ui::success(std::format("loaded {}", p));
    } catch (const std::exception& ex) {
        ui::error(ex.what());
    }
}

void do_save(const e3::plat& world) {
    std::string p = ui::prompt("map json path", "map.json");
    if (p.empty()) {
        ui::info("cancelled");
        return;
    }
    if (std::filesystem::exists(p) && !ui::confirm("overwrite " + p + "?", false)) return;
    try {
        world.save(p);
        ui::success(std::format("saved {}", p));
    } catch (const std::exception& ex) {
        ui::error(ex.what());
    }
}

void do_list(const e3::plat& world) {
    if (world.cities.empty()) {
        ui::info("no cities");
        return;
    }
    ui::print_separator();
    for (auto& [cid, cp] : world.cities) {
        std::cout << ui::bold << cp->name << ui::reset << "  (city id " << cid << ", " << cp->places.size() << " places)\n";
        for (auto& [pid, pp] : cp->places) std::cout << std::format("    [{:016X}] {} (lat {:.5f}, lon {:.5f}, {} roads)\n", pid, pp->name, pp->lat(), pp->lon(), pp->roads.size());
    }
    ui::print_separator();
    ui::pause();
}

void do_add_city(e3::plat& world, uint32_t& next_cid) {
    std::string name = ui::prompt("city name (empty cancels)");
    if (name.empty()) {
        ui::info("cancelled");
        return;
    }
    try {
        world.set_city(next_cid, name);
        ui::success(std::format("added city {} with id {}", name, next_cid));
        ++next_cid;
    } catch (const std::exception& ex) {
        ui::error(ex.what());
    }
}

void do_add_place(e3::plat& world, uint32_t& next_local) {
    auto cid = pick_city(world, "city keyword");
    if (!cid) return;
    auto cp = world.get_city(*cid);
    if (!cp) {
        ui::error("city vanished");
        return;
    }

    std::string name = ui::prompt("place name (empty cancels)");
    if (name.empty()) {
        ui::info("cancelled");
        return;
    }
    std::string lat_s = ui::prompt("latitude", "0");
    std::string lon_s = ui::prompt("longitude", "0");

    try {
        double lat = std::stod(lat_s);
        double lon = std::stod(lon_s);
        uint64_t pid = e3::combine_u32(*cid, next_local);
        cp->set_place(pid, name, {lat, lon});
        ui::success(std::format("added place {} with id {:016X}", name, pid));
        ++next_local;
    } catch (const std::exception& ex) {
        ui::error(ex.what());
    }
}

void do_add_road(e3::plat& world) {
    auto from = pick_place(world, "from: place keyword");
    if (!from) return;
    auto to = pick_place(world, "to:   place keyword");
    if (!to) return;
    bool bi = ui::confirm("make it bidirectional?", true);
    try {
        double d = bi ? world.add_road(*from, *to), world.add_road(*to, *from) : world.add_road(*from, *to);
        ui::success(std::format("road length {}", e3::distance_to_string(d)));
    } catch (const std::exception& ex) {
        ui::error(ex.what());
    }
}

void do_find_path(const e3::plat& world) {
    auto from = pick_place(world, "from: place keyword");
    if (!from) return;
    auto to = pick_place(world, "to:   place keyword");
    if (!to) return;
    try {
        auto path = world.find_path(*from, *to);
        if (path.empty()) {
            ui::warning("no path found");
            return;
        }
        ui::print_separator();
        world.print_path(path);
        ui::print_separator();
        ui::pause();
    } catch (const std::exception& ex) {
        ui::error(ex.what());
    }
}

void do_search(const e3::plat& world) {
    std::string kw = ui::prompt("keyword (empty cancels)");
    if (kw.empty()) {
        ui::info("cancelled");
        return;
    }
    auto places = world.fuzzy_find_places(kw);
    auto cities = world.fuzzy_find_cities(kw);
    ui::print_separator();
    if (cities.empty() && places.empty()) {
        ui::info("no match");
        return;
    }
    for (auto& [id, name] : cities) std::cout << std::format("  city   [{}] {}\n", id, name);
    for (auto& [id, name] : places) std::cout << std::format("  place  [{:016X}] {}\n", id, name);
    ui::print_separator();
    ui::pause();
}

void run_e3() {
    ui::print_title("e3 - city map");
    e3::plat world;
    uint32_t next_cid = 1;
    uint32_t next_local = 1;

    for (;;) {
        int c = ui::select(
            "action", {
                          "load map",
                          "save map",
                          "list everything",
                          "add city",
                          "add place",
                          "add road",
                          "find path",
                          "search",
                          "back to main menu",
                      });
        if (c < 0 || c == 8) return;
        switch (c) {
            case 0:
                do_load(world);
                break;
            case 1:
                do_save(world);
                break;
            case 2:
                do_list(world);
                break;
            case 3:
                do_add_city(world, next_cid);
                break;
            case 4:
                do_add_place(world, next_local);
                break;
            case 5:
                do_add_road(world);
                break;
            case 6:
                do_find_path(world);
                break;
            case 7:
                do_search(world);
                break;
        }
    }
}

int main() {
    ui::print_title("toolkit");
    for (;;) {
        int c = ui::select(
            "choose module", {
                                 "e1 - expression evaluator",
                                 "e2 - huffman coder",
                                 "e3 - city map",
                                 "quit",
                             });
        switch (c) {
            case 0:
                run_e1();
                break;
            case 1:
                run_e2();
                break;
            case 2:
                run_e3();
                break;
            default:
                return 0;
        }
    }
}