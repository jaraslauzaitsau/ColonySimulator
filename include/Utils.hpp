// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "Drawing.hpp"
#include <atomic>
#include <cstdlib>
#include <raygui.h>
#include <raylib.h>
#include <string>
#include <thread>

inline float GetRandomFloat(float a, float b) { return rand() * 1.0f / RAND_MAX * (b - a) + a; }

inline void DrawText(const char* text, Vector2 pos, int fontSize, Color color)
{
    DrawTextEx(myFont, text, pos, fontSize, myFontSpacing, color);
}

template <typename Func, typename... Args>
void ShowLoadingScreen(bool showProgressbar, Func&& f, Args&&... args)
{
    static_assert(
        std::is_invocable_v<Func, std::string&, float&, std::atomic<bool>&, Args...>,
        "Function must accept (std::string& label, float& loadingPercent, std::atomic<bool>& finished, ...) as its arguments.");

    std::string label = "Loading...";
    float loadingPercent = 0;

    std::atomic<bool> finished(false);
    std::thread thread(std::forward<Func>(f), std::ref(label), std::ref(loadingPercent),
                       std::ref(finished), std::forward<Args>(args)...);
    thread.detach();

    while (!finished)
    {
        BeginDrawing();

        ClearBackground(BLACK);

        UpdateWindowSize();

        float fontSize = 24;
        DrawText(label.c_str(), {0, windowSize.y - fontSize}, fontSize, WHITE);

        if (showProgressbar)
        {
            Rectangle progressRec = {windowSize.x, windowSize.y, windowSize.x / 2, fontSize};
            progressRec.x -= progressRec.width;
            progressRec.y -= progressRec.height;
            GuiProgressBar(progressRec, "", "", &loadingPercent, 0, 100);
        }

        EndDrawing();
    }

    ReloadIslandShaderValues();
}
