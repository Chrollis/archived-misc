#include "core.hpp"
#include <format>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <queue>
#include <stdexcept>
#include <unordered_set>

namespace e3 {
place::place(uint64_t id, const std::string& name, const point& globe) : id(id), name(name), globe(globe) {
    plane = wgs84_to_utm(globe.y, globe.x);
}

std::shared_ptr<place> city::get_place(uint64_t id) const {
    auto it = places.find(id);
    return it != places.end() ? it->second : nullptr;
}
place& city::set_place(uint64_t id, const std::string& name, const point& globe) {
    places[id] = std::make_shared<place>(id, name, globe);
    return *places[id];
}
bool city::remove_place(uint64_t id) {
    for (auto& [_, p] : places) {
        p->remove_road(id);
    }
    return places.erase(id);
}

double city::add_road(uint64_t from, uint64_t to) const {
    auto fp = get_place(from);
    auto tp = get_place(to);
    if (fp && tp) {
        return fp->set_road(to, tp->plane);
    }
    throw std::invalid_argument(std::format("place {} does not exist in this city", fp ? to : from));
}
double city::add_biroad(uint64_t from, uint64_t to) const {
    return add_road(from, to), add_road(to, from);
}
double city::add_interroad(uint64_t from, uint64_t to, const point& plane) const {
    auto fp = get_place(from);
    if (fp) {
        return fp->set_road(to, plane);
    }
    throw std::invalid_argument(std::format("place {} does not exist in this city", from));
}

bool city::has_road(uint64_t from, uint64_t to) const {
    auto fp = get_place(from);
    return fp && fp->has_road(to);
}
double city::road_len(uint64_t from, uint64_t to) const {
    auto fp = get_place(from);
    return fp ? fp->road_len(to) : 0.0;
}

std::shared_ptr<place> plat::get_place(uint64_t id) const {
    auto c = get_city(id >> 32);
    if (c) {
        return c->get_place(id);
    }
    return nullptr;
}
double plat::add_road(uint64_t from, uint64_t to) const {
    uint32_t fcid = static_cast<uint32_t>(from >> 32);
    uint32_t tcid = static_cast<uint32_t>(to >> 32);
    auto fc = get_city(fcid);
    auto tc = get_city(tcid);

    if (fc) {
        if (fcid == tcid)
            return fc->add_road(from, to);
        else {
            auto tp = get_place(to);
            if (tp) {
                return fc->add_interroad(from, to, tp->plane);
            }
        }
    }

    return 0.0;
}
std::shared_ptr<city> plat::get_city(uint32_t id) const {
    auto it = cities.find(id);
    return it == cities.end() ? nullptr : it->second;
}
city& plat::set_city(uint32_t id, const std::string& name) {
    cities[id] = std::make_shared<city>(id, name);
    return *cities[id];
}
bool plat::remove_city(uint32_t id) {
    if (!has_city(id)) return 0;
    auto c = get_city(id);
    for (auto& [_, t] : cities) {
        if (t == c) continue;
        for (auto& [_, p] : c->places) {
            t->remove_place(p->id);
        }
    }
    return cities.erase(id);
}

std::vector<uint64_t> plat::find_path(uint64_t from, uint64_t to) const {
    auto sp = get_place(from);
    auto gp = get_place(to);
    if (sp && gp) return astar_search(*sp, *gp);
    return {};
}
void plat::plat::print_path(const std::vector<uint64_t>& path) const {
    if (path.empty()) {
        std::cout << "The path is empty\n";
        return;
    }
    double sum = 0;
    auto curr = get_place(path[0]);
    if (!curr) {
        throw std::runtime_error(std::format("The path contains an unknown place {} as beginning", path[0]));
    }
    std::cout << "Route: " << curr->name;
    for (uint64_t i = 1; i < path.size(); ++i) {
        auto next = get_place(path[i]);
        if (!next) {
            throw std::runtime_error(std::format("The path contains an unknown place {}", path[i]));
        }
        double dist = curr->road_len(next->id);
        std::cout << (next->has_road(curr->id) ? " <-" : " =-") << distance_to_string(dist) << "-> " << next->name;
        curr = next;
        sum += dist;
    }
    std::cout << ", arrived. Totally " << distance_to_string(sum) << "\n";
}
std::vector<uint64_t> plat::astar_search(const place& start, const place& goal) const {
    std::priority_queue<astar_node, std::vector<astar_node>, std::greater<>> open_list;
    std::unordered_map<uint64_t, astar_node> all_nodes;
    std::unordered_map<astar_node, uint64_t> reverse_all_nodes;
    std::unordered_set<uint64_t> closed_set;

    double h_start = astar_node::heuristic(start.plane, goal.plane);
    open_list.push({0.0, h_start, 0});
    all_nodes[start.id] = open_list.top();
    reverse_all_nodes[open_list.top()] = start.id;

    while (!open_list.empty()) {
        astar_node curr = open_list.top();
        open_list.pop();
        uint64_t curr_id = reverse_all_nodes[curr];
        if (curr_id == goal.id) {
            return reconstruct_path(all_nodes, goal.id);
        }
        if (closed_set.count(curr_id)) continue;
        closed_set.insert(curr_id);
        auto curr_place = get_place(curr_id);
        if (!curr_place) continue;

        for (auto& [nb_id, dist] : curr_place->roads) {
            if (closed_set.count(nb_id)) continue;
            auto nb_place = get_place(nb_id);
            if (!nb_place) continue;
            double g = all_nodes[curr_id].g + dist;
            double h = astar_node::heuristic(nb_place->plane, goal.plane);
            if (!all_nodes.count(nb_id) || g < all_nodes[nb_id].g) {
                double f = g + h;
                all_nodes[nb_id] = {g, f, curr_id};
                reverse_all_nodes[all_nodes[nb_id]] = nb_id;
                open_list.push(all_nodes[nb_id]);
            }
        }
    }

    return {};
}
std::vector<uint64_t> plat::reconstruct_path(const std::unordered_map<uint64_t, astar_node>& nodes, uint64_t end_id) const {
    std::vector<uint64_t> path;
    uint64_t curr_id = end_id;
    while (curr_id != 0) {
        path.push_back(curr_id);
        auto it = nodes.find(curr_id);
        if (it == nodes.end()) break;
        curr_id = it->second.parent;
    }
    std::reverse(path.begin(), path.end());
    return path;
}

std::vector<std::pair<uint64_t, std::string>> plat::fuzzy_find_places(const std::string& keyword) const {
    std::vector<std::pair<uint64_t, std::string>> results;
    if (keyword.empty()) {
        return {};
    }
    std::string lower_keyword = keyword;
    std::transform(lower_keyword.begin(), lower_keyword.end(), lower_keyword.begin(), ::tolower);
    for (const auto& [cid, cp] : cities) {
        for (const auto& [pid, pp] : cp->places) {
            std::string lower_name = pp->name + cp->name;
            std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::tolower);
            if (lower_name.find(lower_keyword) != std::string::npos) {
                results.emplace_back(pid, pp->name + ", " + cp->name);
            }
        }
    }
    std::sort(results.begin(), results.end(), [](const auto& a, const auto& b) { return a.second < b.second; });
    return results;
}
std::vector<std::pair<uint32_t, std::string>> plat::fuzzy_find_cities(const std::string& keyword) const {
    std::vector<std::pair<uint32_t, std::string>> results;
    if (keyword.empty()) {
        return {};
    }
    std::string lower_keyword = keyword;
    std::transform(lower_keyword.begin(), lower_keyword.end(), lower_keyword.begin(), ::tolower);
    for (const auto& [cid, cp] : cities) {
        std::string lower_name = cp->name;
        std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::tolower);
        if (lower_name.find(lower_keyword) != std::string::npos) {
            results.emplace_back(cid, cp->name);
        }
    }
    std::sort(results.begin(), results.end(), [](const auto& a, const auto& b) { return a.second < b.second; });
    return results;
}
void plat::save(const std::filesystem::path& path) const {
    nlohmann::json root;

    root["cities"] = nlohmann::json::array();
    for (const auto& [cid, cp] : cities) {
        nlohmann::json cjson;
        cjson["id"] = cid;
        cjson["name"] = cp->name;

        cjson["places"] = nlohmann::json::array();
        for (const auto& [pid, pp] : cp->places) {
            nlohmann::json pjson;
            pjson["id"] = pid;
            pjson["name"] = pp->name;
            pjson["lon"] = pp->globe.y;
            pjson["lat"] = pp->globe.x;
            cjson["places"].push_back(pjson);
        }

        cjson["roads"] = nlohmann::json::array();
        for (const auto& [fid, fp] : cp->places) {
            for (const auto& [tid, dist] : fp->roads) {
                nlohmann::json rjson;
                rjson["from"] = fid;
                rjson["to"] = tid;
                cjson["roads"].push_back(rjson);
            }
        }
        root["cities"].push_back(cjson);
    }
    std::ofstream file(path);
    if (!file.is_open()) throw std::runtime_error(std::format("Failed to open file for saving: {}", path.string()));
    file << root.dump(2) << "\n";
}
void plat::load(const std::filesystem::path& path) {
    std::ifstream file(path);
    if (!file.is_open()) throw std::runtime_error(std::format("Failed to open file for loading: {}", path.string()));
    nlohmann::json root = nlohmann::json::parse(file);
    cities.clear();

    std::vector<std::pair<uint64_t, uint64_t>> deferred_roads;
    for (const auto& cjson : root["cities"]) {
        uint32_t cid = cjson["id"].get<uint32_t>();
        std::string cname = cjson["name"].get<std::string>();
        auto ccity = set_city(cid, cname);

        for (const auto& pjson : cjson["places"]) {
            uint64_t pid = pjson["id"].get<uint64_t>();
            std::string pname = pjson["name"].get<std::string>();
            double lon = pjson["lon"].get<double>();
            double lat = pjson["lat"].get<double>();
            ccity.set_place(pid, pname, {lat, lon});
        }

        for (const auto& rjson : cjson["roads"]) {
            uint64_t from = rjson["from"].get<uint64_t>();
            uint64_t to = rjson["to"].get<uint64_t>();
            uint32_t fcid = static_cast<uint32_t>(from >> 32);
            uint32_t tcid = static_cast<uint32_t>(to >> 32);

            if (fcid != tcid) {
                deferred_roads.push_back({from, to});
            } else {
                ccity.add_road(from, to);
            }
        }
    }

    for (auto [from, to] : deferred_roads) {
        add_road(from, to);
    }
}
}  // namespace e3