// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "Drawing.hpp"
#include "Settings.hpp"
#include "UI.hpp"
#include <raygui.h>

bool showIslandsBoxes = false;

#define UI_SPACING 30
#define ELEMENT_SIZE 30
#define ELEMENT_SPACING 10

bool isSettings = false;

float nextElementPositionY = UI_SPACING * 2;

void DrawCheckBox(const char* text, bool* value)
{
    GuiCheckBox(Rectangle{UI_SPACING * 2, nextElementPositionY, ELEMENT_SIZE, ELEMENT_SIZE}, text,
                value);
    nextElementPositionY += ELEMENT_SIZE + ELEMENT_SPACING;
}

void DrawSettings(bool* isOpen)
{
    if (!*isOpen) return;
    DrawRectangleRounded(Rectangle{UI_SPACING, UI_SPACING, windowSize.x - UI_SPACING * 2,
                                   windowSize.y - UI_SPACING * 2},
                         0.1f, 1, Color{128, 128, 128, 128});
    nextElementPositionY = UI_SPACING * 2;
    DrawCheckBox("vsync", &vsync);
    DrawCheckBox("show-fps", &showFPS);
}

void DrawUI()
{
    if (showFPS) DrawFPS(0, 0);

    if (GuiButton(Rectangle{windowSize.x - ELEMENT_SIZE, 0, ELEMENT_SIZE, ELEMENT_SIZE}, "#142#"))
        isSettings = !isSettings;

    // if (GuiButton(Rectangle{windowSize.x - ELEMENT_SIZE * 2, 0, ELEMENT_SIZE, ELEMENT_SIZE},
    //               "#140#"))
    //     showIslandsBoxes = !showIslandsBoxes;

    DrawSettings(&isSettings);
}
