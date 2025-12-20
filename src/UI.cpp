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
#define UI_SPACING 30
#define ELEMENT_SIZE 30
#define ELEMENT_SPACING 10
#define SLIDER_WIDTH windowSize.x - 270
#define BUTTON_WIDTH ELEMENT_SIZE * 10 * windowSize.x / startWindowSize.x
#define FONT_SIZE 24

bool isSettings = false;
int islandEditIdx = -1;

float nextElementPositionY = UI_SPACING * 2;

void DrawCheckBox(const char* text, bool* value)
{
    GuiCheckBox(Rectangle{UI_SPACING * 2, nextElementPositionY, ELEMENT_SIZE, ELEMENT_SIZE}, text,
                value);
    nextElementPositionY += ELEMENT_SIZE + ELEMENT_SPACING;
}

void DrawSliderInt(const char* leftText, const char* rightText, int* value, float minValue,
                   float maxValue)
{
    float valueFloat = *value;
    GuiSlider({UI_SPACING * 2, nextElementPositionY, SLIDER_WIDTH, ELEMENT_SIZE}, leftText,
              rightText, &valueFloat, minValue, maxValue);
    *value = valueFloat;
    DrawText(std::to_string(*value).c_str(), (SLIDER_WIDTH + UI_SPACING * 2) / 2.f,
             nextElementPositionY + 5, FONT_SIZE, WHITE);
    nextElementPositionY += ELEMENT_SIZE + ELEMENT_SPACING;
}

void DrawTextCentered(const char* text, int fontSize)
{
    fontSize = fontSize * windowSize.y / startWindowSize.y;
    DrawText(text, (windowSize.x - MeasureText(text, fontSize)) / 2, nextElementPositionY, fontSize,
             WHITE);
    nextElementPositionY += fontSize + ELEMENT_SPACING * windowSize.y / startWindowSize.y;
}

int DrawButtonCentered(const char* text)
{
    float buttonHieght = ELEMENT_SIZE * windowSize.y / startWindowSize.y;
    int res = GuiButton(
        {windowSize.x / 2 - BUTTON_WIDTH / 2.0f, nextElementPositionY, BUTTON_WIDTH, buttonHieght},
        text);
    nextElementPositionY += buttonHieght + ELEMENT_SPACING * windowSize.y / startWindowSize.y;
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
        buttonRec.width = ELEMENT_SIZE;
        buttonRec.height = ELEMENT_SIZE;
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
        buttonRec.width = ELEMENT_SIZE;
        buttonRec.height = ELEMENT_SIZE;
        buttonRec.x += rec.width - UI_SPACING;
        if (GuiButton(buttonRec, "#113#")) islandEditIdx = -1;
    }

    auto& island = islands[islandEditIdx];
    DrawSliderInt("", "Podatki", &island.taxes, 0, 100);
}

void DrawGameUI()
{
    if (showFPS) DrawFPS(0, 0);

    if (GuiButton(Rectangle{windowSize.x - ELEMENT_SIZE, 0, ELEMENT_SIZE, ELEMENT_SIZE}, "#142#"))
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
    nextElementPositionY += (ELEMENT_SIZE + ELEMENT_SPACING) * 2 * windowSize.y / startWindowSize.y;
    if (DrawButtonCentered("Play")) OpenGameMenu();
    if (DrawButtonCentered("Settings")) isSettings = !isSettings;
    if (DrawButtonCentered("About")) std::cout << "[TODO]: Add an about menu\n";

    if (isSettings) DrawSettings();
}
