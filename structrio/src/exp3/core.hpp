#pragma once

#include <filesystem>
#include <memory>
#include <unordered_map>
#include "utils.hpp"

namespace e3 {
struct place {
    uint64_t id;
    std::string name;
    point globe;
    point plane;
    std::unordered_map<uint64_t, double> roads;

    place(uint64_t id, const std::string& name, const point& globe);
    double lon() const { return globe.y; }
    double lat() const { return globe.x; }

    double set_road(uint64_t id, const point& plane) { return roads[id] = this->plane.dist(plane); }
    bool remove_road(uint64_t id) { return roads.erase(id); }
    bool has_road(uint64_t id) const { return roads.count(id); }
    double road_len(uint64_t id) { return roads[id]; }
};

struct city {
    uint32_t id;
    std::string name;
    std::unordered_map<uint64_t, std::shared_ptr<place>> places;

    std::shared_ptr<place> get_place(uint64_t id) const;
    bool has_place(uint64_t id) const { return places.count(id); }
    place& set_place(uint64_t id, const std::string& name, const point& globe);
    bool remove_place(uint64_t id);

    double add_road(uint64_t from, uint64_t to) const;
    double add_biroad(uint64_t from, uint64_t to) const;
    double add_interroad(uint64_t from, uint64_t to, const point& plane) const;

    bool has_road(uint64_t from, uint64_t to) const;
    double road_len(uint64_t from, uint64_t to) const;
};

struct plat {
    std::unordered_map<uint32_t, std::shared_ptr<city>> cities;

    struct astar_node {
        static double heuristic(const point& a, const point& b) { return a.dist(b); }
        double g, f;
        uint64_t parent;
        bool operator>(const astar_node& p) const { return f > p.f; }
        bool operator==(const astar_node& p) const { return g == p.g && f == p.f && parent == p.parent; }
    };

    std::shared_ptr<place> get_place(uint64_t id) const;
    double add_road(uint64_t from, uint64_t to) const;
    std::shared_ptr<city> get_city(uint32_t id) const;
    city& set_city(uint32_t id, const std::string& name);
    bool has_city(uint32_t id) const { return cities.count(id); }
    bool remove_city(uint32_t id);

    std::vector<uint64_t> find_path(uint64_t from, uint64_t to) const;
    void print_path(const std::vector<uint64_t>& path) const;
    std::vector<uint64_t> astar_search(const place& start, const place& goal) const;
    std::vector<uint64_t> reconstruct_path(const std::unordered_map<uint64_t, astar_node>& nodes, uint64_t end_id) const;

    std::vector<std::pair<uint64_t, std::string>> fuzzy_find_places(const std::string& keyword) const;
    std::vector<std::pair<uint32_t, std::string>> fuzzy_find_cities(const std::string& keyword) const;
    void save(const std::filesystem::path& path) const;
    void load(const std::filesystem::path& path);
};
}  // namespace e3

namespace std {
template <>
struct hash<e3::plat::astar_node> {
    uint64_t operator()(const e3::plat::astar_node& a) const { return a.parent << 11 ^ (static_cast<uint64_t>(a.g) >> 23 & static_cast<uint64_t>(a.g) << 7); }
};
}  // namespace std
