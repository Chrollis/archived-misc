#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <objidl.h>
#include <windows.h>

#include <gdiplus.h>
#include <algorithm>
#include <cmath>
#include <ctime>
#include <filesystem>
#include <format>
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

inline std::wstring GetExeDir() {
    std::vector<WCHAR> buf(MAX_PATH);
    DWORD len = GetModuleFileNameW(nullptr, buf.data(), static_cast<DWORD>(buf.size()));
    while (len > 0 && len == buf.size()) {
        buf.resize(buf.size() * 2);
        len = GetModuleFileNameW(nullptr, buf.data(), static_cast<DWORD>(buf.size()));
    }
    std::filesystem::path exePath(std::wstring(buf.data(), len));
    return exePath.parent_path().wstring();
}

inline std::string ToUtf8(const std::wstring& w) {
    if (w.empty()) return "";
    const int len = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string s(static_cast<size_t>(len) - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, &s[0], len, nullptr, nullptr);
    return s;
}

inline std::wstring ToWide(const std::string& s) {
    if (s.empty()) return L"";
    const int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
    std::wstring w(static_cast<size_t>(len) - 1, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, &w[0], len);
    return w;
}

inline std::wstring ResolvePath(const std::string& path) {
    const std::wstring w = ToWide(path);
    return (std::filesystem::path(GetExeDir()) / path).lexically_normal().wstring();
}

struct MonitorInfo {
    std::wstring device;
    bool primary = false;
    int left = 0, top = 0, width = 0, height = 0;
};

inline BOOL CALLBACK EnumMonitorsProc(HMONITOR hMon, HDC, LPRECT, LPARAM lParam) {
    auto* out = reinterpret_cast<std::vector<MonitorInfo>*>(lParam);
    MONITORINFOEXW mi = {};
    mi.cbSize = sizeof(mi);
    if (GetMonitorInfoW(hMon, &mi)) {
        MonitorInfo info;
        info.device = mi.szDevice;
        info.primary = (mi.dwFlags & MONITORINFOF_PRIMARY) != 0;
        info.left = mi.rcMonitor.left;
        info.top = mi.rcMonitor.top;
        info.width = mi.rcMonitor.right - mi.rcMonitor.left;
        info.height = mi.rcMonitor.bottom - mi.rcMonitor.top;
        out->push_back(info);
    }
    return TRUE;
}

inline std::vector<MonitorInfo> EnumMonitors() {
    std::vector<MonitorInfo> list;
    EnumDisplayMonitors(nullptr, nullptr, EnumMonitorsProc, reinterpret_cast<LPARAM>(&list));
    return list;
}

inline bool ResolveMonitor(const std::string& deviceName, MonitorInfo& out) {
    const std::vector<MonitorInfo> monitors = EnumMonitors();
    if (monitors.empty()) return false;
    if (!deviceName.empty()) {
        for (const MonitorInfo& m : monitors)
            if (ToUtf8(m.device) == deviceName) {
                out = m;
                return true;
            }
    }
    for (const MonitorInfo& m : monitors)
        if (m.primary) {
            out = m;
            return true;
        }
    out = monitors.front();
    return true;
}

inline bool keepLastLogs(size_t line) {
    std::error_code ec;
    std::wstring path = ResolvePath("carross.log");
    auto size = std::filesystem::file_size(path, ec);
    if (ec || line == 0 || size == 0) return !ec;

    std::ifstream in(path, std::ios::binary);
    if (!in) return false;
    char lastCh = 0;
    in.seekg(static_cast<std::streamoff>(size) - 1);
    in.read(&lastCh, 1);
    const bool lastIsNewline = (lastCh == '\n');

    const size_t target = line + (lastIsNewline ? 1u : 0u);
    constexpr size_t CHUNK = 8192;
    std::vector<char> buf(CHUNK);

    std::uintmax_t end = size;
    std::uintmax_t cutPos = 0;
    std::size_t cnt = 0;
    bool found = false;

    while (end > 0 && !found) {
        const std::uintmax_t start = (end > CHUNK) ? end - CHUNK : 0;
        const std::size_t len = static_cast<std::size_t>(end - start);
        in.clear();
        in.seekg(static_cast<std::streamoff>(start));
        in.read(buf.data(), static_cast<std::streamsize>(len));
        const std::size_t got = static_cast<std::size_t>(in.gcount());
        for (std::size_t i = got; i-- > 0;) {
            if (buf[i] == '\n') {
                if (++cnt == target) {
                    cutPos = start + i + 1;
                    found = true;
                    break;
                }
            }
        }
        end = start;
    }

    in.close();
    if (!found) return true;
    std::filesystem::resize_file(path, cutPos, ec);
    return !ec;
}

inline void LogLine(const std::string& line) {
    char ts[32] = {};
    std::time_t now = std::time(nullptr);
    std::tm tmv = {};
    localtime_s(&tmv, &now);
    std::strftime(ts, sizeof(ts), "[%Y-%m-%d %H:%M:%S]", &tmv);
    std::ofstream f(ResolvePath("carross.log"), std::ios::app);
    if (f) {
        f << ts << "  " << line << "\n";
    }
}

class GdiplusStartupGuard {
public:
    GdiplusStartupGuard() { Gdiplus::GdiplusStartup(&token_, &input_, nullptr); }
    ~GdiplusStartupGuard() { Gdiplus::GdiplusShutdown(token_); }

private:
    ULONG_PTR token_ = 0;
    Gdiplus::GdiplusStartupInput input_;
};
static GdiplusStartupGuard g_gdiplus;

struct ImageCache {
    std::wstring path;
    ULONGLONG mtime = 0;
    std::vector<uint32_t> pixels;
    int w = 0, h = 0;
    bool loaded = false;
};
static ImageCache g_cache;

inline bool LoadImagePixels(const std::wstring& path, ImageCache& out) {
    Gdiplus::Bitmap bmp(path.c_str());
    if (bmp.GetLastStatus() != Gdiplus::Ok) return false;
    const int iw = static_cast<int>(bmp.GetWidth());
    const int ih = static_cast<int>(bmp.GetHeight());
    if (iw <= 0 || ih <= 0) return false;

    Gdiplus::Rect rect(0, 0, iw, ih);
    Gdiplus::BitmapData data = {};
    if (bmp.LockBits(&rect, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &data) != Gdiplus::Ok) return false;

    out.pixels.assign(static_cast<size_t>(iw) * ih, 0);
    const char* base = static_cast<const char*>(data.Scan0);
    for (int y = 0; y < ih; ++y) {
        std::memcpy(out.pixels.data() + static_cast<size_t>(y) * iw, base + static_cast<size_t>(y) * data.Stride, static_cast<size_t>(iw) * 4);
    }
    bmp.UnlockBits(&data);
    out.w = iw;
    out.h = ih;
    return true;
}

inline uint32_t SampleBilinear(double x, double y, const ImageCache& c) {
    if (x < 0 || y < 0 || x >= c.w || y >= c.h) return 0;
    const int x0 = static_cast<int>(x);
    const int y0 = static_cast<int>(y);
    if (x0 >= c.w - 1 || y0 >= c.h - 1) return c.pixels[static_cast<size_t>(y0) * c.w + x0];  // last row/col -> nearest

    const double fx = x - x0;
    const double fy = y - y0;
    const uint32_t p00 = c.pixels[static_cast<size_t>(y0) * c.w + x0];
    const uint32_t p10 = c.pixels[static_cast<size_t>(y0) * c.w + x0 + 1];
    const uint32_t p01 = c.pixels[static_cast<size_t>(y0 + 1) * c.w + x0];
    const uint32_t p11 = c.pixels[static_cast<size_t>(y0 + 1) * c.w + x0 + 1];
    const double w00 = (1.0 - fx) * (1.0 - fy);
    const double w10 = fx * (1.0 - fy);
    const double w01 = (1.0 - fx) * fy;
    const double w11 = fx * fy;

    auto lerp = [&](int shift) -> int {
        const int a = (p00 >> shift) & 0xFF, b = (p10 >> shift) & 0xFF;
        const int c0 = (p01 >> shift) & 0xFF, d = (p11 >> shift) & 0xFF;
        return static_cast<int>(a * w00 + b * w10 + c0 * w01 + d * w11);
    };
    const int a = lerp(24), r = lerp(16), g = lerp(8), b = lerp(0);
    return (static_cast<uint32_t>(a) << 24) | (static_cast<uint32_t>(r) << 16) | (static_cast<uint32_t>(g) << 8) | static_cast<uint32_t>(b);
}

inline void BlendPixel(uint32_t* dst, uint32_t src) {
    const int sa = (src >> 24) & 0xFF;
    if (sa == 0) return;
    const int da = (*dst >> 24) & 0xFF;
    const int oa = sa + da * (255 - sa) / 255;
    if (oa == 0) {
        *dst = 0;
        return;
    }
    const int sr = (src >> 16) & 0xFF, sg = (src >> 8) & 0xFF, sb = src & 0xFF;
    const int dr = (*dst >> 16) & 0xFF, dg = (*dst >> 8) & 0xFF, db = *dst & 0xFF;
    const int r = (sr * sa + dr * da * (255 - sa) / 255) / oa;
    const int g = (sg * sa + dg * da * (255 - sa) / 255) / oa;
    const int b = (sb * sa + db * da * (255 - sa) / 255) / oa;
    *dst = (static_cast<uint32_t>(oa) << 24) | (static_cast<uint32_t>(r) << 16) | (static_cast<uint32_t>(g) << 8) | static_cast<uint32_t>(b);
}

enum class CrossStyle {
    Circle,
    Cannon,
    Line,
    None,
    Image,
    Double,
};

struct Config {
    unsigned char r = 0;
    unsigned char g = 255;
    unsigned char b = 0;
    unsigned char center_r = 0;
    unsigned char center_g = 255;
    unsigned char center_b = 0;
    CrossStyle style = CrossStyle::Circle;
    int cross_radius = 40;
    int cross_interval = 8;
    int cross_thickness = 2;
    int center_radius = 2;
    int screen_dx = 0;
    int screen_dy = 0;
    double cross_scale = 1.0;
    double cross_angle = 0.0;
    std::string image;
    std::string language = "en";
    std::string monitor;
};

inline int GetInt(const nlohmann::json& j, const char* key, int def) {
    auto it = j.find(key);
    if (it != j.end() && it->is_number_integer()) {
        return it->get<int>();
    }
    return def;
}

inline void ParseColor(const nlohmann::json& j, const char* key, unsigned char& r, unsigned char& g, unsigned char& b) {
    auto it = j.find(key);
    if (it == j.end() || !it->is_string()) {
        return;
    }
    std::string s = it->get<std::string>();
    if (!s.empty() && s[0] == '#') {
        s.erase(s.begin());
    }
    if (s.size() != 6) {
        return;
    }
    char* end = nullptr;
    unsigned long v = std::strtoul(s.c_str(), &end, 16);
    if (end != s.c_str() + 6) {
        return;
    }
    r = static_cast<unsigned char>((v >> 16) & 0xFF);
    g = static_cast<unsigned char>((v >> 8) & 0xFF);
    b = static_cast<unsigned char>(v & 0xFF);
}

inline void ParseStyle(const nlohmann::json& j, CrossStyle& style) {
    auto it = j.find("style");
    if (it == j.end() || !it->is_string()) {
        return;
    }
    std::string s = it->get<std::string>();
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    if (s == "line") {
        style = CrossStyle::Line;
    } else if (s == "none") {
        style = CrossStyle::None;
    } else if (s == "circle") {
        style = CrossStyle::Circle;
    } else if (s == "cannon") {
        style = CrossStyle::Cannon;
    } else if (s == "image") {
        style = CrossStyle::Image;
    } else if (s == "double") {
        style = CrossStyle::Double;
    }
}

inline bool LoadConfig(Config& out) {
    out = Config{};
    const std::wstring path = ResolvePath("config.json");

    std::ifstream file(path);
    if (!file.is_open()) {
        return true;
    }
    nlohmann::json j;
    try {
        file >> j;
    } catch (...) {
        return true;
    }

    ParseColor(j, "color", out.r, out.g, out.b);
    ParseColor(j, "center_color", out.center_r, out.center_g, out.center_b);
    ParseStyle(j, out.style);

    out.cross_radius = std::max(1, GetInt(j, "cross_radius", out.cross_radius));
    out.cross_interval = std::max(0, GetInt(j, "cross_interval", out.cross_interval));
    out.cross_thickness = std::max(1, GetInt(j, "cross_thickness", out.cross_thickness));
    out.center_radius = std::max(0, GetInt(j, "center_radius", out.center_radius));
    out.screen_dx = GetInt(j, "screen_dx", out.screen_dx);
    out.screen_dy = GetInt(j, "screen_dy", out.screen_dy);

    if (j.contains("cross_scale") && j["cross_scale"].is_number()) {
        out.cross_scale = std::max(0.1, j["cross_scale"].get<double>());
    }
    if (j.contains("cross_angle") && j["cross_angle"].is_number()) {
        out.cross_angle = j["cross_angle"].get<double>();
    }
    if (j.contains("image") && j["image"].is_string()) {
        out.image = j["image"].get<std::string>();
    }
    if (j.contains("language") && j["language"].is_string()) {
        const std::string lang = j["language"].get<std::string>();
        if (lang == "zh" || lang == "en") out.language = lang;
    }
    if (j.contains("monitor") && j["monitor"].is_string()) {
        out.monitor = j["monitor"].get<std::string>();
    }
    return true;
}

inline bool SaveConfig(const Config& cfg) {
    std::string color = std::format("#{:02X}{:02X}{:02X}", cfg.r, cfg.g, cfg.b);
    std::string centerColor = std::format("#{:02X}{:02X}{:02X}", cfg.center_r, cfg.center_g, cfg.center_b);
    nlohmann::json j;
    j["color"] = color;
    j["center_color"] = centerColor;

    switch (cfg.style) {
        case CrossStyle::Circle:
            j["style"] = "circle";
            break;
        case CrossStyle::Cannon:
            j["style"] = "cannon";
            break;
        case CrossStyle::Line:
            j["style"] = "line";
            break;
        case CrossStyle::None:
            j["style"] = "none";
            break;
        case CrossStyle::Image:
            j["style"] = "image";
            break;
        case CrossStyle::Double:
            j["style"] = "double";
            break;
    }
    j["cross_radius"] = cfg.cross_radius;
    j["cross_interval"] = cfg.cross_interval;
    j["cross_thickness"] = cfg.cross_thickness;
    j["center_radius"] = cfg.center_radius;
    j["screen_dx"] = cfg.screen_dx;
    j["screen_dy"] = cfg.screen_dy;
    j["cross_scale"] = cfg.cross_scale;
    j["cross_angle"] = cfg.cross_angle;
    j["image"] = cfg.image;
    j["language"] = cfg.language;
    j["monitor"] = cfg.monitor;

    const std::wstring path = ResolvePath("config.json");
    std::ofstream f(path);
    if (!f.is_open()) {
        return false;
    }
    f << j.dump(2) << "\n";
    return true;
}

inline bool RenderImage(const Config& cfg, int cx, int cy, int width, int height, uint32_t* pixels) {
    if (cfg.image.empty()) return false;

    const std::wstring path = ResolvePath(cfg.image);

    ULONGLONG mtime = 0;
    {
        WIN32_FILE_ATTRIBUTE_DATA d = {};
        if (!GetFileAttributesExW(path.c_str(), GetFileExInfoStandard, &d)) return false;
        mtime = (static_cast<ULONGLONG>(d.ftLastWriteTime.dwHighDateTime) << 32) | d.ftLastWriteTime.dwLowDateTime;
    }
    if (!g_cache.loaded || g_cache.path != path || g_cache.mtime != mtime) {
        ImageCache fresh;
        fresh.path = path;
        fresh.mtime = mtime;
        if (!LoadImagePixels(path, fresh)) {
            LogLine("failed to load image: " + cfg.image);
            g_cache.loaded = false;
            return false;
        }
        fresh.loaded = true;
        g_cache = std::move(fresh);
        LogLine("image loaded: " + cfg.image);
    }

    const double scale = cfg.cross_scale > 0.0 ? cfg.cross_scale : 1.0;
    const double pi = 3.14159265358979323846;
    const double rad = cfg.cross_angle * pi / 180.0;
    const double ca = std::cos(rad);
    const double sa = std::sin(rad);
    const double iW = static_cast<double>(g_cache.w);
    const double iH = static_cast<double>(g_cache.h);
    const double dW = iW * scale;
    const double dH = iH * scale;

    const int x0 = static_cast<int>(cx - dW / 2.0 - 1.0);
    const int x1 = static_cast<int>(cx + dW / 2.0 + 1.0);
    const int y0 = static_cast<int>(cy - dH / 2.0 - 1.0);
    const int y1 = static_cast<int>(cy + dH / 2.0 + 1.0);

    for (int ty = y0; ty <= y1; ++ty) {
        if (ty < 0 || ty >= height) continue;
        for (int tx = x0; tx <= x1; ++tx) {
            if (tx < 0 || tx >= width) continue;
            const double dx = tx - cx;
            const double dy = ty - cy;
            const double rx = dx * ca + dy * sa;
            const double ry = -dx * sa + dy * ca;
            const double sx = rx / scale + iW / 2.0;
            const double sy = ry / scale + iH / 2.0;
            const uint32_t color = SampleBilinear(sx, sy, g_cache);
            if (((color >> 24) & 0xFF) == 0) continue;
            BlendPixel(&pixels[static_cast<size_t>(ty) * width + tx], color);
        }
    }
    return true;
}

inline void FillCircle(uint32_t* pixels, int width, int height, int cx, int cy, int r, uint32_t color) {
    const long r2 = static_cast<long>(r) * r;
    for (int dy = -r; dy <= r; ++dy)
        for (int dx = -r; dx <= r; ++dx)
            if (static_cast<long>(dx) * dx + static_cast<long>(dy) * dy <= r2) {
                const int x = cx + dx, y = cy + dy;
                if (x >= 0 && x < width && y >= 0 && y < height) pixels[y * width + x] = color;
            }
}

inline void DrawRing(uint32_t* pixels, int width, int height, int cx, int cy, int radius, int thickness, uint32_t color) {
    const double loD = static_cast<double>(radius) - thickness / 2.0;
    const double hiD = static_cast<double>(radius) + (thickness - 1) / 2.0;
    const long lo2 = static_cast<long>(loD * loD);
    const long hi2 = static_cast<long>(hiD * hiD);
    const int pad = thickness / 2 + 1;
    for (int dy = -radius - pad; dy <= radius + pad; ++dy)
        for (int dx = -radius - pad; dx <= radius + pad; ++dx) {
            const long d2 = static_cast<long>(dx) * dx + static_cast<long>(dy) * dy;
            if (d2 >= lo2 && d2 <= hi2) {
                const int x = cx + dx, y = cy + dy;
                if (x >= 0 && x < width && y >= 0 && y < height) pixels[y * width + x] = color;
            }
        }
}

inline void DrawCrossLines(uint32_t* pixels, int width, int height, int cx, int cy, int radius, int interval, int thickness, double angleDeg, uint32_t color) {
    const double pi = 3.14159265358979323846;
    const double rad = angleDeg * pi / 180.0;
    const double half = thickness / 2.0;
    const int pad = radius + thickness;
    for (int k = 0; k < 4; ++k) {
        const double a = rad + k * (pi / 2.0);
        const double ca = std::cos(a);
        const double sa = std::sin(a);
        for (int dy = -pad; dy <= pad; ++dy)
            for (int dx = -pad; dx <= pad; ++dx) {
                const double proj = dx * ca + dy * sa;
                const double perp = dx * -sa + dy * ca;
                if (proj >= interval && proj <= radius && perp >= -half && perp < half) {
                    const int x = cx + dx, y = cy + dy;
                    if (x >= 0 && x < width && y >= 0 && y < height) pixels[y * width + x] = color;
                }
            }
    }
}

inline void RenderCrosshair(const Config& cfg, int width, int height, uint32_t* pixels) {
    const int cx = width / 2 + cfg.screen_dx;
    const int cy = height / 2 + cfg.screen_dy;
    const uint32_t crossColor = (0xFFu << 24) | (cfg.r << 16) | (cfg.g << 8) | cfg.b;
    const uint32_t centerColor = (0xFFu << 24) | (cfg.center_r << 16) | (cfg.center_g << 8) | cfg.center_b;

    const double s = cfg.cross_scale > 0.0 ? cfg.cross_scale : 1.0;
    const int radius = std::max(1, static_cast<int>(cfg.cross_radius * s));
    const int interval = static_cast<int>(cfg.cross_interval * s);
    const int thickness = std::max(1, static_cast<int>(cfg.cross_thickness * s));
    const int center = static_cast<int>(cfg.center_radius * s);

    if (cfg.style == CrossStyle::Image) {
        if (!RenderImage(cfg, cx, cy, width, height, pixels) && center > 0) {
            FillCircle(pixels, width, height, cx, cy, center, centerColor);
        }
        return;
    }

    if (cfg.style == CrossStyle::Circle || cfg.style == CrossStyle::Cannon || cfg.style == CrossStyle::Double) {
        DrawRing(pixels, width, height, cx, cy, radius, thickness, crossColor);
    }
    if (cfg.style == CrossStyle::Double) {
        DrawRing(pixels, width, height, cx, cy, interval, thickness, crossColor);
    }

    if (cfg.style == CrossStyle::Cannon || cfg.style == CrossStyle::Line) {
        DrawCrossLines(pixels, width, height, cx, cy, radius, interval, thickness, cfg.cross_angle, crossColor);
    }

    if (center > 0) {
        FillCircle(pixels, width, height, cx, cy, center, centerColor);
    }
}