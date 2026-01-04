// SPDX-FileCopyrightText: 2025 SemkiShow
// SPDX-FileContributor: Jaraslau Zaitsau
//
// SPDX-License-Identifier: GPL-3.0-only

#include "Drawing.hpp"
#include "Drawing/GameMenu.hpp"
#include "Human.hpp"
#include "Island.hpp"
#include "Perlin.hpp"
#include "Settings.hpp"
#include "UI.hpp"
#include <algorithm>
#include <iostream>
#include <raygui.h>
#include <raymath.h>

double growthTimer = 0;
Vector2 lastMousePosition = GetMousePosition();
Vector2 mousePressedStart = GetMousePosition();

void OpenGameMenu() { currentMenu = Menu::Game; }

void DrawResources()
{
    // Constants
    const float margin = 10, woodScale = 0.03f, ironScale = 0.03f, humanScale = 0.015f,
                textScale = 24;

    // Calculate the offset
    Vector2 offset = {margin, margin};

    auto GetTextOffset = [&](const Texture& texture, float textureScale) -> Vector2
    {
        Vector2 newOffset = offset;
        newOffset.x += fmax(woodTexture.width * woodScale, ironTexture.width * ironScale) + margin;
        newOffset.y += (texture.height * textureScale - textScale) / 2;
        return newOffset;
    };

    // Draw a dark background for better text visibility
    DrawRectangleRounded({0, 0,
                          fmax(woodTexture.width * woodScale, ironTexture.width * ironScale) * 2.5f,
                          woodTexture.height * woodScale + ironTexture.height * ironScale +
                              humanTexture.height * humanScale + margin * 5},
                         0.25f, 16, {0, 0, 0, 127});

    // Draw wood
    DrawTextureEx(woodTexture, offset, 0, woodScale, WHITE);
    {
        Vector2 textOffset = GetTextOffset(woodTexture, woodScale);
        DrawTextCustom(std::to_string(woodTotal).c_str(), textOffset, textScale, WHITE);
    }
    offset.y += woodTexture.height * woodScale + margin;

    // Draw iron
    DrawTextureEx(ironTexture, offset, 0, ironScale, WHITE);
    {
        Vector2 textOffset = GetTextOffset(ironTexture, ironScale);
        DrawTextCustom(std::to_string(ironTotal).c_str(), textOffset, textScale, WHITE);
    }
    offset.y += ironTexture.height * ironScale + margin;

    // Draw people
    DrawTextureEx(humanTexture, offset + Vector2{humanTexture.width * humanScale / 2, 0}, 0,
                  humanScale, WHITE);
    {
        Vector2 textOffset = GetTextOffset(humanTexture, humanScale);
        DrawTextCustom(std::to_string(peopleTotal).c_str(), textOffset, textScale, WHITE);
    }
    offset.y += humanTexture.height * humanScale + margin;
}

void UpdateDynamicShaderValues()
{
    float scale = perlinScale;
    SetShaderValue(biomeShader, GetShaderLocation(biomeShader, "uScale"), &scale,
                   SHADER_UNIFORM_FLOAT);
    SetShaderValue(islandShader, GetShaderLocation(islandShader, "uScale"), &scale,
                   SHADER_UNIFORM_FLOAT);

    windowSize = {(float)GetRenderWidth(), (float)GetRenderHeight()};
    SetShaderValue(biomeShader, GetShaderLocation(biomeShader, "uResolution"), (float*)&windowSize,
                   SHADER_UNIFORM_VEC2);
    SetShaderValue(islandShader, GetShaderLocation(islandShader, "uResolution"),
                   (float*)&windowSize, SHADER_UNIFORM_VEC2);
    windowSize /= GetWindowScaleDPI();

    SetShaderValue(biomeShader, GetShaderLocation(biomeShader, "uOffset"), (float*)&perlinOffset,
                   SHADER_UNIFORM_VEC2);
    SetShaderValue(islandShader, GetShaderLocation(islandShader, "uOffset"), (float*)&perlinOffset,
                   SHADER_UNIFORM_VEC2);

    SetShaderValue(biomeShader, GetShaderLocation(biomeShader, "uMapSize"), (float*)&mapSize,
                   SHADER_UNIFORM_VEC2);

    Vector2 mousePosition = RaylibToGlsl(GetMousePosition());
    SetShaderValue(islandShader, GetShaderLocation(islandShader, "uMouse"), (float*)&mousePosition,
                   SHADER_UNIFORM_VEC2);
}

void DrawGameMenu()
{
    UpdateDynamicShaderValues();

    BeginShaderMode(biomeShader);
    DrawRectangle(0, 0, windowSize.x, windowSize.y, WHITE);
    EndShaderMode();

    for (auto& human: people)
    {
        if (currentMenu == Menu::Game) human.MoveToTarget(GetFrameTime());
        float scale = 0.0005f / perlinScale;
        Vector2 pos = GlslToRaylib(human.pos);
        DrawTexturePro(humanTexture, {0, 0, humanTexture.width * 1.0f, humanTexture.height * 1.0f},
                       {pos.x, pos.y, humanTexture.width * scale, humanTexture.height * scale},
                       {humanTexture.width * scale / 2.0f, humanTexture.height * scale},
                       human.angle, WHITE);
    }

    if (showIslandsBoxes)
    {
        BeginShaderMode(islandShader);
        DrawRectangle(0, 0, windowSize.x, windowSize.y, WHITE);
        EndShaderMode();
    }

    for (auto& island: islands)
    {
        island.DrawStats();
    }

    DrawResources();

    // Joke feature: Snow (obviously)
    {
        auto mouse = GetMousePosition();
        auto glslMouse = RaylibToGlsl(mouse);
        if (GetPerlin(glslMouse) >= biomes[6].startLevel)
        {
            float fontSize = 24;
            const char* text = "Snow (obviously)";
            DrawRectangleRounded({mouse.x - 3, mouse.y - 3,
                                  MeasureTextEx(myFont, text, fontSize, myFontSpacing).x + 6, 30},
                                 0.25f, 16, {0, 0, 0, 127});
            DrawTextCustom(text, mouse, fontSize, WHITE);
        }
    }

    DrawGameUI();
}

void ProcessPlayerInput(double deltaTime)
{
    if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W))
        perlinOffset.y += panSensitivity * perlinScale * deltaTime;
    if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S))
        perlinOffset.y -= panSensitivity * perlinScale * deltaTime;
    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))
        perlinOffset.x -= panSensitivity * perlinScale * deltaTime;
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D))
        perlinOffset.x += panSensitivity * perlinScale * deltaTime;

    if (!IsWindowFocused() || IsWindowHidden())
    {
        currentMenu = Menu::Pause;
    }

    if (IsKeyPressed(KEY_ESCAPE))
    {
        if (isSettings)
            isSettings = false;
        else
            currentMenu = Menu::Pause;
    }

    float wheelMove = GetMouseWheelMove();
    if (wheelMove > 0) perlinScale -= wheelSensitivity * wheelMove * deltaTime;
    if (wheelMove < 0) perlinScale -= wheelSensitivity * wheelMove * deltaTime;
    perlinScale = std::max(0.0f, perlinScale);

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        mousePressedStart = GetMousePosition();
    }

    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) &&
        Vector2Distance(GetMousePosition(), mousePressedStart) == 0)
    {
        std::cout << "Mouse pressed!\n";
        Vector2 v = RaylibToGlsl(GetMousePosition());
        for (size_t i = 0; i < islands.size(); i++)
        {
            Vector2 offset{4, 2};
            Vector2 center = (islands[i].p1 + islands[i].p2) / 2 - offset;
            const Vector2 boxSize = Vector2{17, 15};
            if (v.x >= center.x - boxSize.x / 2 && v.x <= center.x + boxSize.x / 2 &&
                v.y >= center.y - boxSize.y / 2 && v.y <= center.y + boxSize.y / 2)
            {
                std::cout << "Clicked on island with id: " << i << '\n';
                if (islands[i].colonized)
                    islands[i].SendPeople(1);
                else
                    islands[i].Colonize();
                break;
            }
        }
    }

    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
    {
        Vector2 delta = GetMousePosition() - lastMousePosition;
        delta.y *= -1;
        perlinOffset -= delta * perlinScale * GetWindowScaleDPI();
    }

    if (GetTime() - growthTimer > GROWTH_PERIOD)
    {
        growthTimer = GetTime();
        for (auto& island: islands)
        {
            island.GrowthTick();
        }
    }

    lastMousePosition = GetMousePosition();
}
