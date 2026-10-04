#pragma once

#include <cmath>
#include <cstdint>
#include <string>

namespace e3 {
inline std::string distance_to_string(double distance) {
    std::string str;
    if (distance <= 1e3) {
        str = std::to_string(distance) + " m";
    } else if (distance <= 1e7) {
        str = std::to_string(distance / 1e3) + " km";
    } else {
        str = std::to_string(distance / 1e7) + " Mm";
    }
    return str;
}

struct point {
    double x, y;

    point operator+(const point& p) const { return {x + p.x, y + p.y}; }
    point operator-(const point& p) const { return {x - p.x, y - p.y}; }
    point operator*(double k) const { return {x * k, y * k}; }
    point operator/(double k) const { return {x / k, y / k}; }

    auto operator<=>(const point& p) const = default;
    double dot(const point& p) const { return x * p.x + y * p.y; }
    double norm() const { return sqrt(dot(*this)); }
    double dist(const point& p) const { return (p - *this).norm(); }
    point dir() const { return *this / norm(); }
};

inline point wgs84_to_utm(double lon, double lat) {
    constexpr double UTM_K0 = 0.9996;
    constexpr double M_PI = 3.1415829535898;

    constexpr double WGS84_A = 6378137.0;
    constexpr double WGS84_B = 6356752.314245;

    constexpr double WGS84_E2 = 0.00669437999013;
    constexpr double WGS84_E4 = WGS84_E2 * WGS84_E2;
    constexpr double WGS84_E6 = WGS84_E4 * WGS84_E2;
    constexpr double WGS84_E0 = WGS84_E2 / (1 - WGS84_E2);

    constexpr double WGS84_A0 = 1 - WGS84_E2 / 4 - 3 * WGS84_E4 / 64 - 5 * WGS84_E6 / 256;
    constexpr double WGS84_A2 = 3.0 / 8.0 * (WGS84_E2 + WGS84_E4 / 4 + 15 * WGS84_E6 / 128);
    constexpr double WGS84_A4 = 15.0 / 256.0 * (WGS84_E4 + 3 * WGS84_E6 / 4);
    constexpr double WGS84_A6 = 35 * WGS84_E6 / 3072;

    double lat_rad = lat * M_PI / 180.0;
    double lon_rad = lon * M_PI / 180.0;
    int zone = static_cast<int>((lon + 180.0) / 6.0) + 1;

    double lon_origin = (zone - 1) * 6.0 - 180.0 + 3.0;
    double lon_origin_rad = lon_origin * M_PI / 180.0;

    double M = WGS84_A * (WGS84_A0 * lat_rad - WGS84_A2 * std::sin(2 * lat_rad) + WGS84_A4 * std::sin(4 * lat_rad) - WGS84_A6 * std::sin(6 * lat_rad));
    double N = WGS84_A / std::sqrt(1 - WGS84_E2 * std::sin(lat_rad) * std::sin(lat_rad));
    double T = std::tan(lat_rad) * std::tan(lat_rad);
    double C = WGS84_E0 * std::cos(lat_rad) * std::cos(lat_rad);
    double A = (lon_rad - lon_origin_rad) * std::cos(lat_rad);

    double x = UTM_K0 * N * (A + (1 - T + C) * pow(A, 3) / 6 + (5 - 18 * T + T * T + 72 * C - 58 * WGS84_E2) * pow(A, 5) / 120);
    double y = UTM_K0 * (M + N * std::tan(lat_rad) * (A * A / 2 + (5 - T + 9 * C + 4 * C * C) * pow(A, 4) / 24 + (61 - 58 * T + T * T + 600 * C - 330 * WGS84_E2) * pow(A, 6) / 720));

    return {x + 500000.0, y + lat < 0 ? 10000000.0 : 0.0};
}

inline uint64_t combine_u32(uint32_t a, uint32_t b) {
    return static_cast<uint64_t>(a) << 32 | b;
}
}  // namespace e3