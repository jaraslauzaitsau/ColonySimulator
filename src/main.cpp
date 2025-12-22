// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "Drawing.hpp"
#include "Progress.hpp"
#include "Settings.hpp"
#include <raygui.h>

int main()
{
    Load();
    LoadProgress();

    int flags = 0;
    if (vsync) flags |= FLAG_VSYNC_HINT;
    flags |= FLAG_WINDOW_HIGHDPI;
    flags |= FLAG_WINDOW_RESIZABLE;
    SetConfigFlags(flags);
    GuiSetStyle(DEFAULT, TEXT_SIZE, 24);

    InitWindow(windowSize.x, windowSize.y, "Colony Simulator");
    SetExitKey(-1);

    GuiSetFont(GetFontDefault());

    InitGPU();

    while (!WindowShouldClose())
    {
        DrawFrame();
    }

    Save();
    SaveProgress();
    FreeResources();
    CloseWindow();

    return 0;
}
