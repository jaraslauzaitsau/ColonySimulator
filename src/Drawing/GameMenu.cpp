// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "Drawing.hpp"
#include "Drawing/GameMenu.hpp"
#include "Island.hpp"
#include "Perlin.hpp"
#include "Settings.hpp"
#include "UI.hpp"
#include <algorithm>
#include <iostream>
#include <raygui.h>

double growthTimer = 0;

void OpenGameMenu()
{
    currentMenu = Menu::Game;
}

void DrawStats(int islandIdx)
{
    auto island = islands[islandIdx];

    // Do not draw anything if the scale is too small
    float scale = 0.01f / perlinScale;
    if (scale < 0.05f) return;

    // Constants
    const float margin = 100 * scale, lockScale = 0.5f * scale, woodScale = 0.15f * scale,
                ironScale = 0.15f * scale, humanScale = 0.075f * scale, textScale = 120 * scale,
                buttonScale = 300 * scale;

    // Calculate the center point of the island
    Vector2 center = GlslToRaylib((island.p2 + island.p1) / 2);
    center.x -= lockTexture.width * scale / 2;
    center.y -= lockTexture.height * scale / 2;

    // Calculate the offset
    Vector2 offset = {0, -lockTexture.height * lockScale / 2};
    if (island.colonized) offset *= -1;

    // Draw lock
    if (!island.colonized)
    {
        DrawTextureEx(lockTexture, center + offset, 0, lockScale, WHITE);
        offset.y += lockTexture.height * lockScale + margin;
    }

    auto GetTextOffset = [&](const Texture& texture, float textureScale) -> Vector2
    {
        Vector2 newOffset = center + offset;
        newOffset.x +=
            fmax(woodTexture.width * woodScale, ironTexture.width * ironScale) / 2 + margin;
        newOffset.y += (texture.height * textureScale - textScale) * scale / 2;
        return newOffset;
    };

    // Draw a dark background for better text visibility
    Rectangle rec = {center.x + offset.x - lockTexture.width * lockScale / 2,
                     center.y + offset.y - margin / 2, lockTexture.width * lockScale * 2,
                     woodTexture.height * woodScale + ironTexture.height * ironScale + margin * 2 +
                         (island.colonized ? humanTexture.height * humanScale + margin : 0)};
    DrawRectangleRounded(rec, 0.25f, 16, {0, 0, 0, 127});
    if (!island.colonized) DrawRectangleRoundedLinesEx(rec, 0.25f, 16, 3, RED);

    // Draw wood
    DrawTextureEx(woodTexture, center + offset - Vector2{woodTexture.width * woodScale / 2, 0}, 0,
                  woodScale, WHITE);
    {
        Vector2 textOffset = GetTextOffset(woodTexture, woodScale);
        DrawText(std::to_string(island.colonized ? island.woodCount : island.woodColonize).c_str(),
                 textOffset.x, textOffset.y, textScale, WHITE);
    }
    offset.y += woodTexture.height * woodScale + margin;

    // Draw iron
    DrawTextureEx(ironTexture, center + offset - Vector2{ironTexture.width * ironScale / 2, 0}, 0,
                  ironScale, WHITE);
    {
        Vector2 textOffset = GetTextOffset(ironTexture, ironScale);
        DrawText(std::to_string(island.colonized ? island.ironCount : island.ironColonize).c_str(),
                 textOffset.x, textOffset.y, textScale, WHITE);
    }
    offset.y += ironTexture.height * ironScale + margin;

    // Draw people
    if (island.colonized)
    {
        DrawTextureEx(humanTexture,
                      center + offset - Vector2{humanTexture.width * humanScale / 2, 0}, 0,
                      humanScale, WHITE);
        {
            Vector2 textOffset = GetTextOffset(humanTexture, humanScale);
            DrawText(std::to_string(island.peopleCount).c_str(), textOffset.x, textOffset.y,
                     textScale, WHITE);
        }
        offset.y += humanTexture.height * humanScale + margin;
    }

    // Draw taxes button
    if (island.colonized)
    {
        auto buttonRec = rec;
        buttonRec.width = buttonRec.height = buttonScale;
        buttonRec.x += rec.width;
        if (GuiButton(buttonRec, "#142#")) islandEditIdx = islandIdx;
    }
}

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
        DrawText(std::to_string(woodTotal).c_str(), textOffset.x, textOffset.y, textScale, WHITE);
    }
    offset.y += woodTexture.height * woodScale + margin;

    // Draw iron
    DrawTextureEx(ironTexture, offset, 0, ironScale, WHITE);
    {
        Vector2 textOffset = GetTextOffset(ironTexture, ironScale);
        DrawText(std::to_string(ironTotal).c_str(), textOffset.x, textOffset.y, textScale, WHITE);
    }
    offset.y += ironTexture.height * ironScale + margin;

    // Draw people
    DrawTextureEx(humanTexture, offset + Vector2{humanTexture.width * humanScale / 2, 0}, 0,
                  humanScale, WHITE);
    {
        Vector2 textOffset = GetTextOffset(humanTexture, humanScale);
        DrawText(std::to_string(peopleTotal).c_str(), textOffset.x, textOffset.y, textScale, WHITE);
    }
    offset.y += humanTexture.height * humanScale + margin;
}

void GrowthTick()
{
    for (auto& island: islands)
    {
        island.GrowthTick();
    }
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

    if (showIslandsBoxes)
    {
        BeginShaderMode(islandShader);
        DrawRectangle(0, 0, windowSize.x, windowSize.y, WHITE);
        EndShaderMode();
    }

    for (size_t i = 0; i < islands.size(); i++)
    {
        DrawStats(i);
    }

    DrawResources();

    // Joke feature: Snow (obviously)
    {
        auto mouse = GetMousePosition();
        auto glslMouse = RaylibToGlsl(mouse);
        if (GetPerlin(glslMouse) >= biomes[6].startLevel)
        {
            DrawRectangleRounded({mouse.x - 3, mouse.y - 3, 200, 30}, 0.25f, 16, {0, 0, 0, 127});
            DrawText("Snow (obviously)", mouse.x, mouse.y, 24, WHITE);
        }
    }

    DrawGameUI();
}

void ProcessPlayerInput(double deltaTime)
{
    if (IsKeyDown(KEY_UP)) perlinOffset.y += panSensitivity * perlinScale * deltaTime;
    if (IsKeyDown(KEY_DOWN)) perlinOffset.y -= panSensitivity * perlinScale * deltaTime;
    if (IsKeyDown(KEY_LEFT)) perlinOffset.x -= panSensitivity * perlinScale * deltaTime;
    if (IsKeyDown(KEY_RIGHT)) perlinOffset.x += panSensitivity * perlinScale * deltaTime;

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

    if (GetTime() - growthTimer > GROWTH_PERIOD)
    {
        growthTimer = GetTime();
        GrowthTick();
    }
}
