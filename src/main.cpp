// SPDX-FileCopyrightText: 2025 SemkiShow
// SPDX-FileContributor: Jaraslau Zaitsau
//
// SPDX-License-Identifier: GPL-3.0-only

#include "Drawing.hpp"
#include "Progress.hpp"
#include "Settings.hpp"
#include <ctime>
#include <raygui.h>

int main()
{
    srand(time(0));

    int flags = 0;
    if (vsync) flags |= FLAG_VSYNC_HINT;
    flags |= FLAG_WINDOW_HIGHDPI;
    flags |= FLAG_WINDOW_RESIZABLE;
    flags |= FLAG_WINDOW_ALWAYS_RUN;
    SetConfigFlags(flags);
    GuiSetStyle(DEFAULT, TEXT_SIZE, 24);

    InitWindow(windowSize.x, windowSize.y, "Colony Simulator");
    SetExitKey(-1);

    GuiSetFont(GetFontDefault());

    {
        auto func = [](std::string& label, float& loadingPercent, std::atomic<bool>& finished)
        {
            label = "Loading progress...";
            loadingPercent = 0;
            Load();
            LoadProgress();
            finished = true;
        };
        ShowLoadingScreen(false, func);
    }

    InitGPU();

    GuiSetFont(myFont);
    GuiSetStyle(DEFAULT, TEXT_SIZE, 20);

    while (!shouldClose && !WindowShouldClose())
    {
        DrawFrame();
    }

    {
        auto func = [](std::string& label, float& loadingPercent, std::atomic<bool>& finished)
        {
            label = "Saving progress...";
            loadingPercent = 0;
            Save();
            SaveProgress();
            finished = true;
        };
        ShowLoadingScreen(false, func);
    }
    FreeResources();
    CloseWindow();

    return 0;
}
