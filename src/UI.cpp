// SPDX-FileCopyrightText: 2025 SemkiShow
// SPDX-FileContributor: Jaraslau Zaitsau
//
// SPDX-License-Identifier: GPL-3.0-only

#include "Drawing.hpp"
#include "Drawing/GameMenu.hpp"
#include "Island.hpp"
#include "Languages.hpp"
#include "Perlin.hpp"
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
#define SLIDER_TEXT_WIDTH (windowSize.x - UI_SPACING * 4) / 3
#define SLIDER_WIDTH (windowSize.x - UI_SPACING * 4 - SLIDER_TEXT_WIDTH)
#define BUTTON_WIDTH ELEMENT_SIZE * 10 * windowSize.x / startWindowSize.x
#define BUTTON_SIZE ELEMENT_SIZE* windowSize.x / startWindowSize.x
#define FONT_SIZE 24 * windowSize.y / startWindowSize.y
#define TEXT_OFFSET 5
#define BUTTON_SPACING ELEMENT_SPACING * 3

float nextElementPositionY = UI_SPACING * 2;

bool isSettings = false;
bool isLoadMap = false;
bool isEmptySlot = false;
bool isNewWorld = false;
bool isSaveGame = false;

int slotToEmpty = -1;
int newMapSlot = -1;
int slotSeed = -1;
bool squareMap = true;
Vector2 slotMapSize{300, 300};
int islandEditIdx = -1;

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
    DrawText(std::to_string(*value).c_str(),
             {(SLIDER_WIDTH + UI_SPACING * 2) / 2.f, nextElementPositionY + TEXT_OFFSET},
             ELEMENT_SIZE - 5, WHITE);
    nextElementPositionY += ELEMENT_SIZE + ELEMENT_SPACING;
}

void DrawSliderInt(const char* leftText, const char* rightText, int* value, float minValue,
                   float maxValue)
{
    float valueFloat = *value;
    GuiSlider({UI_SPACING * 2, nextElementPositionY, SLIDER_WIDTH, ELEMENT_SIZE}, leftText,
              rightText, &valueFloat, minValue, maxValue);
    *value = valueFloat;
    DrawText(std::to_string(*value).c_str(),
             {(SLIDER_WIDTH + UI_SPACING * 2) / 2.f, nextElementPositionY + TEXT_OFFSET},
             ELEMENT_SIZE - TEXT_OFFSET, WHITE);
    nextElementPositionY += ELEMENT_SIZE + ELEMENT_SPACING;
}

void DrawTextCentered(const char* text, int fontSize)
{
    fontSize = fontSize * windowSize.y / startWindowSize.y;
    DrawText(text,
             {(windowSize.x - MeasureTextEx(myFont, text, fontSize, myFontSpacing).x) / 2,
              nextElementPositionY},
             fontSize, WHITE);
    nextElementPositionY += fontSize + ELEMENT_SPACING * windowSize.y / startWindowSize.y;
}

int DrawButtonCentered(const char* text)
{
    float buttonHieght = ELEMENT_SIZE * windowSize.y / startWindowSize.y;
    int res = GuiButton(
        {windowSize.x / 2 - BUTTON_WIDTH / 2.0f, nextElementPositionY, BUTTON_WIDTH, buttonHieght},
        text);
    nextElementPositionY += buttonHieght + BUTTON_SPACING * windowSize.y / startWindowSize.y;
    return res;
}

void DrawValueBox(const char* text, int* value, int minValue, int maxValue)
{
    GuiValueBox(
        {UI_SPACING * 2 + SLIDER_TEXT_WIDTH, nextElementPositionY, SLIDER_WIDTH, ELEMENT_SIZE},
        text, value, minValue, maxValue, true);
    nextElementPositionY += ELEMENT_SIZE + ELEMENT_SPACING;
}

void DrawLanguageButtons(float posX)
{
    for (auto& lang: languages)
    {
        if (GuiButton({posX, nextElementPositionY, ELEMENT_SIZE, ELEMENT_SIZE}, lang.c_str()))
        {
            currentLanguage = lang;
            ReloadLabels();
        }
        posX += ELEMENT_SIZE + ELEMENT_SPACING;
    }
    DrawText(labels["language"].c_str(), {posX, nextElementPositionY}, ELEMENT_SIZE - TEXT_OFFSET,
             WHITE);
}

void DrawSettings()
{
    Rectangle rec = {UI_SPACING, UI_SPACING, windowSize.x - UI_SPACING * 2,
                     windowSize.y - UI_SPACING * 2};
    DrawRectangleRounded(rec, 0.1f, 1, Color{127, 127, 127, 127});
    nextElementPositionY = rec.y + UI_SPACING;

    DrawCheckBox(labels["vsync"].c_str(), &vsync);
    DrawCheckBox(labels["show-fps"].c_str(), &showFPS);
    DrawSlider("", labels["pan-sensitivity"].c_str(), &panSensitivity, 100, 1000);
    DrawSlider("", labels["wheel-sensitivity"].c_str(), &wheelSensitivity, 0.05f, 10);
    DrawLanguageButtons(rec.x + UI_SPACING);

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
    DrawRectangleRounded(rec, 0.1f, 1, Color{127, 127, 127, 127});
    nextElementPositionY = rec.y + UI_SPACING;
    for (size_t i = 0; i < MAX_SAVE_SLOTS; i++)
    {
        float posX = UI_SPACING * 2;
        if (saveSlots[i].seed == -1)
        {
            // New map
            if (GuiButton({posX, nextElementPositionY, BUTTON_SIZE, BUTTON_SIZE}, "+"))
            {
                newMapSlot = i;
                slotSeed = rand();
                squareMap = true;
                slotMapSize = {300, 300};
                isNewWorld = true;
            }
            posX += (BUTTON_SIZE + ELEMENT_SPACING) * 2;
        }
        else
        {
            // Load map
            if (GuiButton({posX, nextElementPositionY, BUTTON_SIZE, BUTTON_SIZE}, "#131#"))
            {
                LoadFromSlot(i);
                OpenGameMenu();
                isLoadMap = false;
            }
            posX += BUTTON_SIZE + ELEMENT_SPACING;
            // Delete map
            if (GuiButton({posX, nextElementPositionY, BUTTON_SIZE, BUTTON_SIZE}, "#143#"))
            {
                isEmptySlot = true;
                slotToEmpty = i;
            }
            posX += BUTTON_SIZE + ELEMENT_SPACING;
        }
        DrawText(saveSlots[i].name.c_str(), {posX, nextElementPositionY}, FONT_SIZE, WHITE);
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
        int res = GuiMessageBox(rec, labels["Warning"].c_str(),
                                (labels["Are you sure you want to empty"] + " " +
                                 std::string(saveSlots[slotToEmpty].name) + "?")
                                    .c_str(),
                                labels["Yes;No"].c_str());
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
    DrawRectangleRounded(rec, 0.1f, 1, Color{127, 127, 127, 127});
    nextElementPositionY = rec.y + UI_SPACING;

    {
        auto buttonRec = rec;
        buttonRec.width = ELEMENT_SIZE;
        buttonRec.height = ELEMENT_SIZE;
        buttonRec.x += rec.width - UI_SPACING;
        if (GuiButton(buttonRec, "#113#"))
        {
            islandEditIdx = -1;
            return;
        }
    }

    auto& island = islands[islandEditIdx];
    DrawSliderInt("", labels["Taxes"].c_str(), &island.taxes, 0, 100);
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

void DrawNewWorld()
{
    Rectangle rec = {UI_SPACING, UI_SPACING, windowSize.x - UI_SPACING * 2,
                     windowSize.y - UI_SPACING * 2};
    DrawRectangleRounded(rec, 0.1f, 1, Color{127, 127, 127, 127});
    nextElementPositionY = rec.y + UI_SPACING;

    Vector2 lastMapSize = slotMapSize;

    DrawValueBox(labels["seed"].c_str(), &slotSeed, 0, 100);
    DrawCheckBox(labels["square map"].c_str(), &squareMap);
    DrawSlider("", labels["map size x"].c_str(), &slotMapSize.x, 50, 1000);
    DrawSlider("", labels["map size y"].c_str(), &slotMapSize.y, 50, 1000);
    if (DrawButtonCentered(labels["Create map"].c_str()))
    {
        isNewWorld = false;
        perlinSeed = slotSeed;
        mapSize = slotMapSize;
        BuildMap();
        SaveToSlot(newMapSlot);
    }

    if (squareMap)
    {
        if (lastMapSize.x != slotMapSize.x) slotMapSize.y = slotMapSize.x;
        if (lastMapSize.y != slotMapSize.y) slotMapSize.x = slotMapSize.y;
    }

    {
        auto buttonRec = rec;
        buttonRec.width = ELEMENT_SIZE * windowSize.x / startWindowSize.x;
        buttonRec.height = ELEMENT_SIZE * windowSize.y / startWindowSize.y;
        buttonRec.x += rec.width - buttonRec.width;
        if (GuiButton(buttonRec, "#113#")) isNewWorld = false;
    }
}

void DrawMainUI()
{
    if (showFPS) DrawFPS(0, 0);

    nextElementPositionY = UI_SPACING;

    DrawTextCentered(labels["Colony Simulator"].c_str(), 48);
    nextElementPositionY += (ELEMENT_SIZE + ELEMENT_SPACING) * 2 * windowSize.y / startWindowSize.y;
    if (DrawButtonCentered(labels["Play"].c_str())) isLoadMap = true;
    if (DrawButtonCentered(labels["Settings"].c_str())) isSettings = !isSettings;
    if (DrawButtonCentered(labels["Exit"].c_str())) shouldClose = true;

    if (isSettings)
        DrawSettings();
    else if (isNewWorld)
        DrawNewWorld();
    else if (isLoadMap)
        DrawLoadMap();
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
    DrawRectangleRounded(rec, 0.1f, 1, Color{127, 127, 127, 127});
    nextElementPositionY = rec.y + UI_SPACING;
    if (DrawButtonCentered(labels["Return to game"].c_str())) OpenGameMenu();
    if (DrawButtonCentered(labels["Save game"].c_str())) SaveProgress();
    if (DrawButtonCentered(labels["Go to the main menu"].c_str())) isSaveGame = true;

    if (isSaveGame)
    {
        int res = GuiMessageBox(rec, labels["Info"].c_str(),
                                labels["Would you like to save the game before exiting?"].c_str(),
                                labels["Yes;No"].c_str());
        if (res >= 0)
        {
            if (res == 1) SaveProgress();
            currentMenu = Menu::Main;
            isSaveGame = false;
        }
    }
}
