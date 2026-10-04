#include "tray.hpp"

#include <commdlg.h>
#include <shellapi.h>
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

constexpr UINT_PTR kTrayIconId = 1;
constexpr UINT_PTR kMenuReload = 1;
constexpr UINT_PTR kMenuOpenConfig = 2;
constexpr UINT_PTR kMenuExit = 3;
constexpr UINT_PTR kMenuStyleCycle = 4;
constexpr UINT_PTR kMenuChooseImage = 5;
constexpr UINT_PTR kMenuParamBase = 10;
constexpr UINT_PTR kMenuLangBase = 20;
constexpr UINT_PTR kMenuColorBase = 30;
constexpr UINT_PTR kMenuMonitorBase = 40;
constexpr UINT_PTR kMenuCenterColorBase = 50;

static TrayCallbacks g_cb;

HICON LoadTrayIcon(HINSTANCE hInstance) {
    HICON hIcon = LoadIconW(hInstance, MAKEINTRESOURCEW(kAppIconId));
    if (hIcon) {
        LogLine("tray icon loaded from resource");
        return hIcon;
    }
    const std::wstring file = GetExeDir() + L"\\icon.ico";
    hIcon = static_cast<HICON>(LoadImageW(nullptr, file.c_str(), IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE));
    if (hIcon) {
        LogLine("tray icon loaded from file");
        return hIcon;
    }
    LogLine("tray icon: falling back to default");
    return LoadIcon(nullptr, IDI_APPLICATION);
}

const wchar_t* StyleName(bool zh, CrossStyle s) {
    switch (s) {
        case CrossStyle::Circle:
            return zh ? L"圆环" : L"Circle";
        case CrossStyle::Cannon:
            return zh ? L"防空炮" : L"Cannon";
        case CrossStyle::Line:
            return zh ? L"十字" : L"Line";
        case CrossStyle::None:
            return zh ? L"圆点" : L"None";
        case CrossStyle::Image:
            return zh ? L"图片" : L"Image";
        case CrossStyle::Double:
            return zh ? L"双环" : L"Double Ring";
    }
    return L"?";
}

void CycleStyle(int direction, int step) {
    Config& cfg = *g_cb.config;
    constexpr int kStyleCount = static_cast<int>(CrossStyle::Double) + 1;
    int cur = static_cast<int>(cfg.style);
    cur = (cur + direction * step) % kStyleCount;
    if (cur < 0) cur += kStyleCount;
    cfg.style = static_cast<CrossStyle>(cur);
    SaveConfig(cfg);
    g_cb.onConfigWritten();
    g_cb.redraw();
}

struct ParamRange {
    double min;
    double max;
    double step;
};

double ModStep(double cur, const ParamRange& r, bool up) {
    const bool ctrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
    double v = cur;
    if (ctrl)
        v += (up ? 1 : -1) * 10.0 * r.step;
    else
        v += (up ? 1 : -1) * r.step;
    const double k = 1.0 / r.step;
    return std::clamp(std::round(v * k) / k, r.min, r.max);
}

int FindCurrentMonitorIndex(const std::vector<MonitorInfo>& monitors, const Config& cfg) {
    if (!cfg.monitor.empty()) {
        for (size_t i = 0; i < monitors.size(); ++i)
            if (ToUtf8(monitors[i].device) == cfg.monitor) return static_cast<int>(i);
    }
    for (size_t i = 0; i < monitors.size(); ++i)
        if (monitors[i].primary) return static_cast<int>(i);
    return 0;
}

void AdjustParam(int index, int delta) {
    Config& cfg = *g_cb.config;
    const bool up = delta > 0;

    MonitorInfo mon;
    if (!ResolveMonitor(cfg.monitor, mon)) return;
    const double dx = static_cast<double>(mon.width) / 2.0;
    const double dy = static_cast<double>(mon.height) / 2.0;

    switch (index) {
        case 0:
            cfg.cross_radius = static_cast<int>(ModStep(cfg.cross_radius, {1, 1000, 1}, up));
            break;
        case 1:
            cfg.cross_interval = static_cast<int>(ModStep(cfg.cross_interval, {0, 500, 1}, up));
            break;
        case 2:
            cfg.cross_thickness = static_cast<int>(ModStep(cfg.cross_thickness, {1, 100, 1}, up));
            break;
        case 3:
            cfg.center_radius = static_cast<int>(ModStep(cfg.center_radius, {0, 100, 1}, up));
            break;
        case 4:
            cfg.screen_dx = static_cast<int>(ModStep(cfg.screen_dx, {-dx, dx, 1}, up));
            break;
        case 5:
            cfg.screen_dy = static_cast<int>(ModStep(cfg.screen_dy, {-dy, dy, 1}, up));
            break;
        case 6:
            cfg.cross_scale = ModStep(cfg.cross_scale, {0.1, 10.0, 0.1}, up);
            break;
        case 7:
            cfg.cross_angle = ModStep(cfg.cross_angle, {-360.0, 360.0, 1}, up);
            break;
        default:
            return;
    }

    SaveConfig(cfg);
    g_cb.onConfigWritten();
    g_cb.redraw();
}

void AdjustColorChannels(unsigned char* r, unsigned char* g, unsigned char* b, int channel, int delta) {
    unsigned char* p = nullptr;
    switch (channel) {
        case 0:
            p = r;
            break;
        case 1:
            p = g;
            break;
        case 2:
            p = b;
            break;
    }
    if (!p) return;
    *p = static_cast<unsigned char>(ModStep(*p, {0, 255, 1}, delta > 0));
    SaveConfig(*g_cb.config);
    g_cb.onConfigWritten();
    g_cb.redraw();
}

void AdjustColor(int channel, int delta) {
    Config& cfg = *g_cb.config;
    AdjustColorChannels(&cfg.r, &cfg.g, &cfg.b, channel, delta);
}

void AdjustCenterColor(int channel, int delta) {
    Config& cfg = *g_cb.config;
    AdjustColorChannels(&cfg.center_r, &cfg.center_g, &cfg.center_b, channel, delta);
}

void OpenConfigLocation() {
    const std::wstring path = GetExeDir() + L"\\config.json";
    ShellExecuteW(nullptr, L"open", L"explorer.exe", (L"/select,\"" + path + L"\"").c_str(), nullptr, SW_SHOWNORMAL);
}

void ChooseImage() {
    Config& cfg = *g_cb.config;
    wchar_t file[MAX_PATH] = {};
    OPENFILENAMEW ofn = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFilter = L"Image files (*.png;*.bmp;*.jpg;*.jpeg)\0*.png;*.bmp;*.jpg;*.jpeg\0All files (*.*)\0*.*\0";
    ofn.lpstrFile = file;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = L"Choose crosshair image";
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
    if (!GetOpenFileNameW(&ofn)) return;

    cfg.image = ToUtf8(file);
    cfg.style = CrossStyle::Image;
    SaveConfig(cfg);
    g_cb.onConfigWritten();
    g_cb.redraw();
}

HMENU CreateTrayMenu() {
    const Config& cfg = *g_cb.config;
    const bool zh = (cfg.language == "zh");
    const wchar_t* sStyle = zh ? L"样式" : L"Style";
    const wchar_t* sParams = zh ? L"准心参数" : L"Crosshair Params";
    const wchar_t* sPosition = zh ? L"变换" : L"Transform";
    const wchar_t* sLanguage = zh ? L"语言" : L"Language";
    const wchar_t* sReload = zh ? L"重载配置" : L"Reload Config";
    const wchar_t* sOpen = zh ? L"打开配置" : L"Open Config File";
    const wchar_t* sExit = zh ? L"退出" : L"Exit";
    const wchar_t* sChoose = zh ? L"选择图片..." : L"Choose Image...";
    const wchar_t* p0 = zh ? L"半径" : L"cross_radius";
    const wchar_t* p1 = zh ? L"间距" : L"cross_interval";
    const wchar_t* p2 = zh ? L"线宽" : L"cross_thickness";
    const wchar_t* p3 = zh ? L"中心" : L"center_radius";
    const wchar_t* p4 = zh ? L"水平偏移" : L"screen_dx";
    const wchar_t* p5 = zh ? L"垂直偏移" : L"screen_dy";
    const wchar_t* p6 = zh ? L"缩放" : L"cross_scale";
    const wchar_t* p7 = zh ? L"角度" : L"cross_angle";

    HMENU menu = CreatePopupMenu();

    const std::wstring styleItem = std::wstring(sStyle) + L"\t" + StyleName(zh, cfg.style);
    AppendMenuW(menu, MF_STRING, static_cast<UINT_PTR>(kMenuStyleCycle), styleItem.c_str());

    AppendMenuW(menu, MF_STRING, static_cast<UINT_PTR>(kMenuChooseImage), sChoose);
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);

    const wchar_t* cR = zh ? L"红" : L"Red";
    const wchar_t* cG = zh ? L"绿" : L"Green";
    const wchar_t* cB = zh ? L"蓝" : L"Blue";
    const auto appendColorMenu = [&](UINT_PTR base, const wchar_t* title, int rv, int gv, int bv) {
        HMENU cm = CreatePopupMenu();
        AppendMenuW(cm, MF_STRING, static_cast<UINT_PTR>(base + 0), (std::wstring(cR) + L"\t" + std::to_wstring(rv)).c_str());
        AppendMenuW(cm, MF_STRING, static_cast<UINT_PTR>(base + 1), (std::wstring(cG) + L"\t" + std::to_wstring(gv)).c_str());
        AppendMenuW(cm, MF_STRING, static_cast<UINT_PTR>(base + 2), (std::wstring(cB) + L"\t" + std::to_wstring(bv)).c_str());
        AppendMenuW(menu, MF_POPUP, reinterpret_cast<UINT_PTR>(cm), title);
    };
    appendColorMenu(kMenuColorBase, zh ? L"准心颜色" : L"Crosshair Color", cfg.r, cfg.g, cfg.b);
    appendColorMenu(kMenuCenterColorBase, zh ? L"中心颜色" : L"Center Color", cfg.center_r, cfg.center_g, cfg.center_b);

    HMENU params = CreatePopupMenu();
    AppendMenuW(params, MF_STRING, static_cast<UINT_PTR>(kMenuParamBase + 0), (std::wstring(p0) + L"\t" + std::to_wstring(cfg.cross_radius)).c_str());
    AppendMenuW(params, MF_STRING, static_cast<UINT_PTR>(kMenuParamBase + 1), (std::wstring(p1) + L"\t" + std::to_wstring(cfg.cross_interval)).c_str());
    AppendMenuW(params, MF_STRING, static_cast<UINT_PTR>(kMenuParamBase + 2), (std::wstring(p2) + L"\t" + std::to_wstring(cfg.cross_thickness)).c_str());
    AppendMenuW(params, MF_STRING, static_cast<UINT_PTR>(kMenuParamBase + 3), (std::wstring(p3) + L"\t" + std::to_wstring(cfg.center_radius)).c_str());
    AppendMenuW(menu, MF_POPUP, reinterpret_cast<UINT_PTR>(params), sParams);

    HMENU pos = CreatePopupMenu();
    AppendMenuW(pos, MF_STRING, static_cast<UINT_PTR>(kMenuParamBase + 4), (std::wstring(p4) + L"\t" + std::to_wstring(cfg.screen_dx)).c_str());
    AppendMenuW(pos, MF_STRING, static_cast<UINT_PTR>(kMenuParamBase + 5), (std::wstring(p5) + L"\t" + std::to_wstring(cfg.screen_dy)).c_str());
    AppendMenuW(pos, MF_STRING, static_cast<UINT_PTR>(kMenuParamBase + 6), (std::wstring(p6) + L"\t" + std::to_wstring(cfg.cross_scale)).c_str());
    AppendMenuW(pos, MF_STRING, static_cast<UINT_PTR>(kMenuParamBase + 7), (std::wstring(p7) + L"\t" + std::to_wstring(cfg.cross_angle)).c_str());
    AppendMenuW(menu, MF_POPUP, reinterpret_cast<UINT_PTR>(pos), sPosition);

    const wchar_t* sScreen = zh ? L"屏幕" : L"Screen";
    HMENU screenMenu = CreatePopupMenu();
    const std::vector<MonitorInfo> monitors = EnumMonitors();
    const int curMon = FindCurrentMonitorIndex(monitors, cfg);
    for (int i = 0; i < static_cast<int>(monitors.size()); ++i) {
        const MonitorInfo& mi = monitors[i];
        std::wstring label = mi.primary ? (zh ? L"主屏" : L"Primary") : ((zh ? L"显示器 " : L"Monitor ") + std::to_wstring(i + 1));
        label += L" · " + std::to_wstring(mi.width) + L"×" + std::to_wstring(mi.height);
        AppendMenuW(screenMenu, MF_STRING | (i == curMon ? MF_CHECKED : MF_UNCHECKED), static_cast<UINT_PTR>(kMenuMonitorBase + i), label.c_str());
    }
    AppendMenuW(menu, MF_POPUP, reinterpret_cast<UINT_PTR>(screenMenu), sScreen);

    HMENU lang = CreatePopupMenu();
    AppendMenuW(lang, MF_STRING | (zh ? MF_UNCHECKED : MF_CHECKED), static_cast<UINT_PTR>(kMenuLangBase + 0), L"English");
    AppendMenuW(lang, MF_STRING | (zh ? MF_CHECKED : MF_UNCHECKED), static_cast<UINT_PTR>(kMenuLangBase + 1), L"中文");
    AppendMenuW(menu, MF_POPUP, reinterpret_cast<UINT_PTR>(lang), sLanguage);

    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(menu, MF_STRING, static_cast<UINT_PTR>(kMenuReload), sReload);
    AppendMenuW(menu, MF_STRING, static_cast<UINT_PTR>(kMenuOpenConfig), sOpen);
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(menu, MF_STRING, static_cast<UINT_PTR>(kMenuExit), sExit);

    return menu;
}

bool HandleMenuCommand(HWND hwnd, UINT_PTR cmd) {
    const std::vector<MonitorInfo> monitors = EnumMonitors();
    if (cmd >= kMenuMonitorBase && cmd < kMenuMonitorBase + static_cast<UINT_PTR>(monitors.size())) {
        const int idx = static_cast<int>(cmd - kMenuMonitorBase);
        if (idx >= 0 && idx < static_cast<int>(monitors.size())) {
            Config& cfg = *g_cb.config;
            cfg.monitor = ToUtf8(monitors[idx].device);

            const int hw = monitors[idx].width / 2;
            const int hh = monitors[idx].height / 2;
            cfg.screen_dx = std::clamp(cfg.screen_dx, -hw, hw);
            cfg.screen_dy = std::clamp(cfg.screen_dy, -hh, hh);
            SaveConfig(cfg);
            g_cb.onConfigWritten();
            if (g_cb.recreateOverlay) g_cb.recreateOverlay();
        }
        return false;
    }
    if (cmd >= kMenuParamBase && cmd < kMenuParamBase + 8) {
        const int delta = (GetKeyState(VK_SHIFT) & 0x8000) ? -1 : 1;
        AdjustParam(static_cast<int>(cmd - kMenuParamBase), delta);
        return true;
    }
    if (cmd >= kMenuColorBase && cmd < kMenuColorBase + 3) {
        const int delta = (GetKeyState(VK_SHIFT) & 0x8000) ? -1 : 1;
        AdjustColor(static_cast<int>(cmd - kMenuColorBase), delta);
        return true;
    }
    if (cmd >= kMenuCenterColorBase && cmd < kMenuCenterColorBase + 3) {
        const int delta = (GetKeyState(VK_SHIFT) & 0x8000) ? -1 : 1;
        AdjustCenterColor(static_cast<int>(cmd - kMenuCenterColorBase), delta);
        return true;
    }
    if (cmd >= kMenuLangBase && cmd < kMenuLangBase + 2) {
        Config& cfg = *g_cb.config;
        cfg.language = (cmd == kMenuLangBase) ? "en" : "zh";
        SaveConfig(cfg);
        g_cb.onConfigWritten();
        return true;
    }
    switch (cmd) {
        case kMenuStyleCycle: {
            const bool shift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
            const bool ctrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
            CycleStyle(shift ? -1 : 1, ctrl ? 5 : 1);
            return true;
        }
        case kMenuChooseImage:
            ChooseImage();
            return true;
        case kMenuReload:
            g_cb.reload();
            return false;
        case kMenuOpenConfig:
            OpenConfigLocation();
            return false;
        case kMenuExit:
            DestroyWindow(hwnd);
            return false;
        default:
            return false;
    }
}

void ShowTrayMenu(HWND hwnd) {
    SetForegroundWindow(hwnd);
    POINT pt;
    GetCursorPos(&pt);

    for (;;) {
        HMENU menu = CreateTrayMenu();
        const UINT_PTR cmd = TrackPopupMenu(menu, TPM_RIGHTBUTTON | TPM_LEFTALIGN | TPM_RETURNCMD, pt.x, pt.y, 0, hwnd, nullptr);
        DestroyMenu(menu);
        PostMessageW(hwnd, WM_NULL, 0, 0);
        if (cmd == 0) break;
        if (!HandleMenuCommand(hwnd, cmd)) break;
    }
}

void TrayInit(const TrayCallbacks& cb) {
    g_cb = cb;
}

bool TrayAddIcon(HINSTANCE hInstance, HWND owner) {
    NOTIFYICONDATAW nid = {};
    nid.cbSize = sizeof(nid);
    nid.hWnd = owner;
    nid.uID = static_cast<UINT>(kTrayIconId);
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = kTrayMsg;
    nid.hIcon = LoadTrayIcon(hInstance);
    wcscpy_s(nid.szTip, L"Carross");
    return Shell_NotifyIconW(NIM_ADD, &nid) != FALSE;
}

void TrayRemoveIcon(HWND owner) {
    NOTIFYICONDATAW nid = {};
    nid.cbSize = sizeof(nid);
    nid.hWnd = owner;
    nid.uID = static_cast<UINT>(kTrayIconId);
    Shell_NotifyIconW(NIM_DELETE, &nid);
}

bool TrayOnMessage(HWND owner, WPARAM wParam, LPARAM lParam) {
    if (wParam != kTrayIconId) return false;

    if (lParam == WM_RBUTTONUP || lParam == WM_LBUTTONUP || lParam == WM_CONTEXTMENU) {
        ShowTrayMenu(owner);
    }
    return true;
}