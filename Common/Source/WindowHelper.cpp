/*
 * Copyright (c) 2022 Andreas Pohl
 * Licensed under MIT (https://github.com/apohl79/audiogridder/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#include "WindowHelper.hpp"

#if !JUCE_MAC

#ifdef JUCE_WINDOWS
#include <windows.h>
#include <cmath>
#endif

namespace e47 {
namespace WindowHelper {

juce::Rectangle<int> getWindowScreenBounds(juce::Component* c) {
#ifdef JUCE_WINDOWS
    if (c == nullptr) {
        juce::Logger::writeToLog("WindowHelper: cannot get native bounds for a null component");
        return {};
    }

    auto hwnd = static_cast<HWND>(c->getWindowHandle());
    if (hwnd == nullptr || !IsWindow(hwnd)) {
        juce::Logger::writeToLog("WindowHelper: component has no valid native window handle");
        return {};
    }

    // A plugin editor can be hosted by a child HWND. Capture the complete top-level
    // window, but retain the original HWND as a fallback for unusual Wine window trees.
    auto root = GetAncestor(hwnd, GA_ROOT);
    if (root == nullptr || !IsWindow(root)) {
        root = hwnd;
    }

    RECT rect{};
    if (!GetWindowRect(root, &rect) || rect.right <= rect.left || rect.bottom <= rect.top) {
        if (root != hwnd && GetWindowRect(hwnd, &rect) && rect.right > rect.left && rect.bottom > rect.top) {
            root = hwnd;
        } else {
            juce::Logger::writeToLog("WindowHelper: GetWindowRect failed for the plugin editor window");
            return {};
        }
    }

    // Convert native screen coordinates to JUCE's 96-DPI logical coordinate space.
    // Wine may report zero/invalid desktop DPI, so use 96 DPI rather than dividing by zero.
    float dpi = 96.0f;
    if (HDC hDC = GetDC(nullptr)) {
        const int dpiX = GetDeviceCaps(hDC, LOGPIXELSX);
        const int dpiY = GetDeviceCaps(hDC, LOGPIXELSY);
        if (dpiX > 0 && dpiY > 0) {
            dpi = (dpiX + dpiY) / 2.0f;
        }
        ReleaseDC(nullptr, hDC);
    }

    const float scale = 96.0f / dpi;
    const int left = static_cast<int>(std::lround(scale * rect.left));
    const int top = static_cast<int>(std::lround(scale * rect.top));
    const int right = static_cast<int>(std::lround(scale * rect.right));
    const int bottom = static_cast<int>(std::lround(scale * rect.bottom));

    juce::Logger::writeToLog("WindowHelper: native editor window bounds "
                             + juce::String(left) + "x" + juce::String(top) + " "
                             + juce::String(right - left) + "x" + juce::String(bottom - top)
                             + " (DPI " + juce::String(dpi, 1) + ")");

    return {left, top, right - left, bottom - top};
#else
    ignoreUnused(c);
    return {};
#endif
}

}  // namespace WindowHelper
}  // namespace e47

#endif
