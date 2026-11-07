#pragma once
#include "utils.hpp"

struct TrayCallbacks {
    Config* config = nullptr;
    void (*redraw)() = nullptr;
    void (*reload)() = nullptr;
    void (*onConfigWritten)() = nullptr;
    void (*recreateOverlay)() = nullptr;
};

constexpr UINT kTrayMsg = WM_APP + 1;
constexpr int kAppIconId = 101;

void TrayInit(const TrayCallbacks& cb);
bool TrayAddIcon(HINSTANCE hInstance, HWND owner);
void TrayRemoveIcon(HWND owner);
bool TrayOnMessage(HWND owner, WPARAM wParam, LPARAM lParam);
