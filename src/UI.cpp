// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "Drawing.hpp"
#include "Drawing/GameMenu.hpp"
#include "Island.hpp"
#include "Settings.hpp"
#include "UI.hpp"
#include <iostream>
#include <raygui.h>
#include <string>

bool showIslandsBoxes = false;

Vector2 startWindowSize = windowSize;
#define UI_SPACING 30 * windowSize.y / startWindowSize.y
#define ELEMENT_SIZE_X 30 * windowSize.x / startWindowSize.x
#define ELEMENT_SIZE_Y 30 * windowSize.y / startWindowSize.y
#define ELEMENT_SPACING 10 * windowSize.y / startWindowSize.y
#define SLIDER_WIDTH (startWindowSize.x - 270) * windowSize.x / startWindowSize.x
#define BUTTON_WIDTH ELEMENT_SIZE_X * 10

bool isSettings = false;
int islandEditIdx = -1;

float nextElementPositionY = UI_SPACING * 2;

void DrawCheckBox(const char* text, bool* value)
{
    GuiCheckBox(Rectangle{UI_SPACING * 2, nextElementPositionY, ELEMENT_SIZE_X, ELEMENT_SIZE_Y},
                text, value);
    nextElementPositionY += ELEMENT_SIZE_Y + ELEMENT_SPACING;
}

void DrawSliderInt(const char* leftText, const char* rightText, int* value, float minValue,
                   float maxValue)
{
    float fontSize = 24;
    float sliderHeight = fontSize + 7;
    float valueFloat = *value;
    GuiSlider({UI_SPACING * 2, nextElementPositionY, SLIDER_WIDTH, sliderHeight}, leftText,
              rightText, &valueFloat, minValue, maxValue);
    *value = valueFloat;
    DrawText(std::to_string(*value).c_str(), (SLIDER_WIDTH + UI_SPACING * 2) / 2.f,
             nextElementPositionY + 5, fontSize, WHITE);
    nextElementPositionY += sliderHeight + ELEMENT_SPACING;
}

void DrawTextCentered(const char* text, int fontSize)
{
    fontSize = fontSize * windowSize.y / startWindowSize.y;
    DrawText(text, (windowSize.x - MeasureText(text, fontSize)) / 2, nextElementPositionY,
             fontSize, WHITE);
    nextElementPositionY += fontSize + ELEMENT_SPACING;
}

int DrawButtonCentered(const char* text)
{
    int res = GuiButton({windowSize.x / 2 - BUTTON_WIDTH / 2.0f, nextElementPositionY, BUTTON_WIDTH,
                         ELEMENT_SIZE_Y},
                        text);
    nextElementPositionY += ELEMENT_SIZE_Y + ELEMENT_SPACING;
    return res;
}

void DrawSettings()
{
    Rectangle rec = {UI_SPACING, UI_SPACING, windowSize.x - UI_SPACING * 2,
                     windowSize.y - UI_SPACING * 2};
    DrawRectangleRounded(rec, 0.1f, 1, Color{128, 128, 128, 128});
    nextElementPositionY = rec.y + UI_SPACING;
    DrawCheckBox("vsync", &vsync);
    DrawCheckBox("show-fps", &showFPS);

    {
        auto buttonRec = rec;
        buttonRec.width = ELEMENT_SIZE_X;
        buttonRec.height = ELEMENT_SIZE_Y;
        buttonRec.x += rec.width - UI_SPACING;
        if (GuiButton(buttonRec, "#113#")) isSettings = false;
    }
}

void EditIsland()
{
    Rectangle rec = {UI_SPACING, windowSize.y / 2, windowSize.x - UI_SPACING * 2, windowSize.y / 3};
    rec.y -= rec.height / 2;
    DrawRectangleRounded(rec, 0.1f, 1, Color{128, 128, 128, 128});
    nextElementPositionY = rec.y + UI_SPACING;

    {
        auto buttonRec = rec;
        buttonRec.width = ELEMENT_SIZE_X;
        buttonRec.height = ELEMENT_SIZE_Y;
        buttonRec.x += rec.width - UI_SPACING;
        if (GuiButton(buttonRec, "#113#")) islandEditIdx = -1;
    }

    auto& island = islands[islandEditIdx];
    DrawSliderInt("", "Podatki", &island.taxes, 0, 100);
}

void DrawGameUI()
{
    if (showFPS) DrawFPS(0, 0);

    if (GuiButton(Rectangle{windowSize.x - ELEMENT_SIZE_X, 0, ELEMENT_SIZE_X, ELEMENT_SIZE_Y},
                  "#142#"))
        isSettings = !isSettings;

    // if (GuiButton(Rectangle{windowSize.x - ELEMENT_SIZE * 2, 0, ELEMENT_SIZE, ELEMENT_SIZE},
    //               "#140#"))
    //     showIslandsBoxes = !showIslandsBoxes;

    if (islandEditIdx != -1) EditIsland();

    if (isSettings) DrawSettings();
}

void DrawMainUI()
{
    if (showFPS) DrawFPS(0, 0);

    nextElementPositionY = UI_SPACING;

    DrawTextCentered("Colony Simulator", 48);
    nextElementPositionY += (ELEMENT_SIZE_Y + ELEMENT_SPACING) * 2;
    if (DrawButtonCentered("Play")) OpenGameMenu();
    if (DrawButtonCentered("Settings")) isSettings = !isSettings;
    if (DrawButtonCentered("About")) std::cout << "[TODO]: Add an about menu\n";

    if (isSettings) DrawSettings();
}
