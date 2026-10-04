#include "tray.hpp"
#include "utils.hpp"

namespace {
constexpr wchar_t kMainWndClass[] = L"CarrossMainWnd";
constexpr wchar_t kOverlayWndClass[] = L"CarrossOverlayWnd";
constexpr UINT_PTR kRedrawTimerId = 1;
constexpr UINT kRedrawIntervalMs = 16;
constexpr UINT_PTR kFileWatchTimerId = 2;
constexpr UINT kFileWatchIntervalMs = 400;

HINSTANCE g_hInstance = nullptr;
Config g_config;
HWND g_hOverlay = nullptr;
ULONGLONG g_lastConfigWrite = 0;
}  // namespace

void RedrawOverlay(HWND hwnd) {
    RECT rc;
    GetClientRect(hwnd, &rc);
    const int w = rc.right;
    const int h = rc.bottom;
    if (w <= 0 || h <= 0) return;

    HDC hdcScreen = GetDC(hwnd);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);

    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = w;
    bmi.bmiHeader.biHeight = -h;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void* bits = nullptr;
    HBITMAP hbmp = CreateDIBSection(hdcScreen, &bmi, DIB_RGB_COLORS, &bits, nullptr, 0);
    if (!hbmp || !bits) {
        DeleteObject(hbmp);
        DeleteDC(hdcMem);
        ReleaseDC(hwnd, hdcScreen);
        return;
    }

    HBITMAP hOld = static_cast<HBITMAP>(SelectObject(hdcMem, hbmp));
    std::memset(bits, 0, static_cast<size_t>(w) * static_cast<size_t>(h) * 4);
    RenderCrosshair(g_config, w, h, static_cast<uint32_t*>(bits));

    POINT ptDst = {0, 0};
    POINT ptSrc = {0, 0};
    SIZE sz = {w, h};
    BLENDFUNCTION blend = {};
    blend.BlendOp = AC_SRC_OVER;
    blend.SourceConstantAlpha = 255;
    blend.AlphaFormat = AC_SRC_ALPHA;
    UpdateLayeredWindow(hwnd, hdcScreen, &ptDst, &sz, hdcMem, &ptSrc, 0, &blend, ULW_ALPHA);

    SelectObject(hdcMem, hOld);
    DeleteObject(hbmp);
    DeleteDC(hdcMem);
    ReleaseDC(hwnd, hdcScreen);
}

LRESULT CALLBACK OverlayWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            SetTimer(hwnd, kRedrawTimerId, kRedrawIntervalMs, nullptr);
            return 0;
        case WM_TIMER:
            RedrawOverlay(hwnd);
            return 0;
        case WM_NCHITTEST:
            return HTTRANSPARENT;
        case WM_DESTROY:
            KillTimer(hwnd, kRedrawTimerId);
            return 0;
        default:
            return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
}

bool CreateOverlayWindow() {
    MonitorInfo mon;
    if (!ResolveMonitor(g_config.monitor, mon)) {
        mon.left = 0;
        mon.top = 0;
        mon.width = GetSystemMetrics(SM_CXSCREEN);
        mon.height = GetSystemMetrics(SM_CYSCREEN);
    }
    const int left = mon.left;
    const int top = mon.top;
    const int screenW = mon.width;
    const int screenH = mon.height;

    WNDCLASSW wc = {};
    if (!GetClassInfoW(g_hInstance, kOverlayWndClass, &wc)) {
        wc.lpfnWndProc = OverlayWndProc;
        wc.hInstance = g_hInstance;
        wc.lpszClassName = kOverlayWndClass;
        if (!RegisterClassW(&wc)) return false;
    }

    g_hOverlay = CreateWindowExW(
        WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_NOACTIVATE, kOverlayWndClass, L"CarrossOverlay", WS_POPUP, left, top, screenW, screenH, nullptr, nullptr, g_hInstance, nullptr);
    if (!g_hOverlay) return false;

    RedrawOverlay(g_hOverlay);
    ShowWindow(g_hOverlay, SW_SHOWNOACTIVATE);
    return true;
}

void RecreateOverlay() {
    if (g_hOverlay) {
        DestroyWindow(g_hOverlay);
        g_hOverlay = nullptr;
    }
    if (!CreateOverlayWindow()) {
        LogLine("failed to recreate overlay window");
        return;
    }
    LogLine("overlay recreated on configured monitor");
}

void ReloadConfig() {
    LoadConfig(g_config);
    LogLine("config reloaded");
    RedrawOverlay(g_hOverlay);
}

ULONGLONG GetConfigWriteTime() {
    const std::wstring path = ResolvePath("config.json");
    WIN32_FILE_ATTRIBUTE_DATA data = {};
    if (!GetFileAttributesExW(path.c_str(), GetFileExInfoStandard, &data)) return 0;
    return (static_cast<ULONGLONG>(data.ftLastWriteTime.dwHighDateTime) << 32) | data.ftLastWriteTime.dwLowDateTime;
}

bool CheckConfigFileChanged() {
    const ULONGLONG t = GetConfigWriteTime();
    if (t == 0 || t == g_lastConfigWrite) return false;
    g_lastConfigWrite = t;
    return true;
}

void OnTrayRedraw() {
    RedrawOverlay(g_hOverlay);
}
void OnConfigWritten() {
    g_lastConfigWrite = GetConfigWriteTime();
}

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case kTrayMsg:
            TrayOnMessage(hwnd, wParam, lParam);
            return 0;
        case WM_TIMER:
            if (wParam == kFileWatchTimerId && CheckConfigFileChanged()) {
                ReloadConfig();
            }
            return 0;
        case WM_DESTROY:
            TrayRemoveIcon(hwnd);
            PostQuitMessage(0);
            return 0;
        default:
            return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
    g_hInstance = hInstance;
    if (!SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2)) {
        SetProcessDPIAware();
    }

    LoadConfig(g_config);
    SaveConfig(g_config);
    keepLastLogs(1000);
    LogLine(
        "config loaded: r=" + std::to_string(g_config.r) + " g=" + std::to_string(g_config.g) + " b=" + std::to_string(g_config.b) + " style=" + std::to_string(static_cast<int>(g_config.style)) +
        " radius=" + std::to_string(g_config.cross_radius));
    g_lastConfigWrite = GetConfigWriteTime();

    WNDCLASSW wc = {};
    wc.lpfnWndProc = MainWndProc;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(kAppIconId));
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = kMainWndClass;
    if (!RegisterClassW(&wc)) return 1;

    HWND hwndMain = CreateWindowExW(0, kMainWndClass, L"Carross", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr, hInstance, nullptr);
    if (!hwndMain) return 1;

    if (!CreateOverlayWindow()) {
        LogLine("failed to create overlay window");
        return 1;
    }
    LogLine("overlay window created");

    TrayInit(TrayCallbacks{&g_config, OnTrayRedraw, ReloadConfig, OnConfigWritten, RecreateOverlay});
    if (TrayAddIcon(hInstance, hwndMain)) {
        LogLine("tray icon added");
    } else {
        LogLine("failed to add tray icon");
    }

    SetTimer(hwndMain, kFileWatchTimerId, kFileWatchIntervalMs, nullptr);

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return static_cast<int>(msg.wParam);
}
