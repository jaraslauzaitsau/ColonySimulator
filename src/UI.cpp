// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "Drawing.hpp"
#include "Drawing/GameMenu.hpp"
#include "Island.hpp"
#include "Progress.hpp"
#include "Settings.hpp"
#include "UI.hpp"
#include <raygui.h>
#include <string>

bool showIslandsBoxes = false;

Vector2 startWindowSize = windowSize;
#define UI_SPACING 30
#define ELEMENT_SIZE 30
#define ELEMENT_SPACING 10
#define SLIDER_TEXT_WIDTH 150
#define SLIDER_WIDTH (windowSize.x - UI_SPACING * 4 - SLIDER_TEXT_WIDTH)
#define BUTTON_WIDTH ELEMENT_SIZE * 10 * windowSize.x / startWindowSize.x
#define BUTTON_SIZE ELEMENT_SIZE* windowSize.x / startWindowSize.x
#define FONT_SIZE 24 * windowSize.y / startWindowSize.y

bool isSettings = false;
bool isLoadMap = false;
bool isEmptySlot = false;
int slotToEmpty = -1;
bool isSaveGame = false;
int islandEditIdx = -1;

float nextElementPositionY = UI_SPACING * 2;

void DrawCheckBox(const char* text, bool* value)
{
    GuiCheckBox(Rectangle{UI_SPACING * 2, nextElementPositionY, ELEMENT_SIZE, ELEMENT_SIZE}, text,
                value);
    nextElementPositionY += ELEMENT_SIZE + ELEMENT_SPACING;
}

void DrawSlider(const char* leftText, const char* rightText, float* value, float minValue,
                float maxValue)
{
    GuiSlider({UI_SPACING * 2, nextElementPositionY, SLIDER_WIDTH, ELEMENT_SIZE}, leftText,
              rightText, value, minValue, maxValue);
    DrawText(std::to_string(*value).c_str(), (SLIDER_WIDTH + UI_SPACING * 2) / 2.f,
             nextElementPositionY + 5, ELEMENT_SIZE - 5, WHITE);
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
             nextElementPositionY + 5, ELEMENT_SIZE - 5, WHITE);
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
    DrawSlider("", "pan-sensitivity", &panSensitivity, 100, 1000);
    DrawSlider("", "wheel-sensitivity", &wheelSensitivity, 0.05f, 10);

    {
        auto buttonRec = rec;
        buttonRec.width = ELEMENT_SIZE * windowSize.x / startWindowSize.x;
        buttonRec.height = ELEMENT_SIZE * windowSize.y / startWindowSize.y;
        buttonRec.x += rec.width - buttonRec.width;
        if (GuiButton(buttonRec, "#113#")) isSettings = false;
    }
}

void DrawLoadMap()
{
    Rectangle rec = {UI_SPACING, UI_SPACING, windowSize.x - UI_SPACING * 2,
                     windowSize.y - UI_SPACING * 2};
    DrawRectangleRounded(rec, 0.1f, 1, Color{128, 128, 128, 128});
    nextElementPositionY = rec.y + UI_SPACING;
    for (size_t i = 0; i < MAX_SAVE_SLOTS; i++)
    {
        float posX = UI_SPACING * 2;
        if (saveSlots[i].seed == -1)
        {
            if (GuiButton({posX, nextElementPositionY, BUTTON_SIZE, BUTTON_SIZE}, "+"))
            {
                BuildMap();
                SaveToSlot(i);
            }
            posX += (BUTTON_SIZE + ELEMENT_SPACING) * 2;
        }
        else
        {
            if (GuiButton({posX, nextElementPositionY, BUTTON_SIZE, BUTTON_SIZE}, "#131#"))
            {
                LoadFromSlot(i);
                OpenGameMenu();
                isLoadMap = false;
            }
            posX += BUTTON_SIZE + ELEMENT_SPACING;
            if (GuiButton({posX, nextElementPositionY, BUTTON_SIZE, BUTTON_SIZE}, "#143#"))
            {
                isEmptySlot = true;
                slotToEmpty = i;
            }
            posX += BUTTON_SIZE + ELEMENT_SPACING;
        }
        DrawText(saveSlots[i].name.c_str(), posX, nextElementPositionY, FONT_SIZE, WHITE);
        nextElementPositionY += BUTTON_SIZE + ELEMENT_SPACING;
    }

    {
        auto buttonRec = rec;
        buttonRec.width = ELEMENT_SIZE * windowSize.x / startWindowSize.x;
        buttonRec.height = ELEMENT_SIZE * windowSize.y / startWindowSize.y;
        buttonRec.x += rec.width - buttonRec.width;
        if (GuiButton(buttonRec, "#113#")) isLoadMap = false;
    }

    if (isEmptySlot)
    {
        int res = GuiMessageBox(
            rec, "Warning",
            ("Are you sure you want to empty " + saveSlots[slotToEmpty].name + "?").c_str(),
            "Yes;No");
        if (res >= 0)
        {
            if (res == 1) EmptySlot(slotToEmpty);
            isEmptySlot = false;
            slotToEmpty = -1;
        }
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
    DrawSliderInt("", "Taxes", &island.taxes, 0, 100);
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
    if (DrawButtonCentered("Play")) isLoadMap = true;
    if (DrawButtonCentered("Settings")) isSettings = !isSettings;
    if (DrawButtonCentered("Exit")) shouldClose = true;

    if (isSettings) DrawSettings();
    if (isLoadMap) DrawLoadMap();
}

void DrawPauseUI()
{
    if (isSettings)
    {
        DrawSettings();
        return;
    }

    Rectangle rec = {UI_SPACING, UI_SPACING, windowSize.x - UI_SPACING * 2,
                     windowSize.y - UI_SPACING * 2};
    DrawRectangleRounded(rec, 0.1f, 1, Color{128, 128, 128, 128});
    nextElementPositionY = rec.y + UI_SPACING;
    if (DrawButtonCentered("Return to game")) OpenGameMenu();
    if (DrawButtonCentered("Save game")) SaveProgress();
    if (DrawButtonCentered("Go to the main menu")) isSaveGame = true;

    if (isSaveGame)
    {
        int res =
            GuiMessageBox(rec, "Info", "Would you like to save the game before exiting?", "Yes;No");
        if (res >= 0)
        {
            if (res == 1) SaveProgress();
            currentMenu = Menu::Main;
            isSaveGame = false;
        }
    }
}
