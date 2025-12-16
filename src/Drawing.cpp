// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "Drawing.hpp"
#include "Island.hpp"
#include "Perlin.hpp"
#include "Settings.hpp"
#include "UI.hpp"
#include <algorithm>
#include <ctime>
#include <iostream>

Vector2 windowSize{16 * 50, 9 * 50};
double timer = 0;
double growthTimer = 0;
bool lastVsync = vsync;

Shader biomeShader;
Shader islandShader;

Texture lockTexture;
Texture woodTexture;
Texture ironTexture;
Texture humanTexture;

inline Vector2 RaylibToGlsl(Vector2 v)
{
    v.y = windowSize.y - v.y;
    v -= windowSize / 2;
    v *= GetWindowScaleDPI();
    v *= perlinScale;
    v += perlinOffset;
    return v;
}

inline Vector2 GlslToRaylib(Vector2 v)
{
    v.y *= -1;
    v /= GetWindowScaleDPI() * perlinScale;
    v += windowSize / 2;
    v.x -= perlinOffset.x / perlinScale / GetWindowScaleDPI().x;
    v.y += perlinOffset.y / perlinScale / GetWindowScaleDPI().y;
    return v;
}

void InitGPU()
{
    lockTexture = LoadTexture("resources/lock.png");
    woodTexture = LoadTexture("resources/wooden_log.png");
    ironTexture = LoadTexture("resources/iron_ingot.png");
    humanTexture = LoadTexture("resources/human.png");

    biomeShader = LoadShader(0, "resources/Perlin.fs");

    int biomeCount = (int)biomes.size();
    SetShaderValue(biomeShader, GetShaderLocation(biomeShader, "uBiomeCount"), &biomeCount,
                   SHADER_UNIFORM_INT);

    perlinSeed = time(0);
    SetShaderValue(biomeShader, GetShaderLocation(biomeShader, "uSeed"), &perlinSeed,
                   SHADER_UNIFORM_FLOAT);

    {
        float starts[8];
        for (size_t i = 0; i < biomes.size(); i++)
            starts[i] = biomes[i].startLevel;

        SetShaderValueV(biomeShader, GetShaderLocation(biomeShader, "uBiomeStart"), starts,
                        SHADER_UNIFORM_FLOAT, biomeCount);
    }

    {
        float colors[8 * 4];
        for (size_t i = 0; i < biomes.size(); i++)
        {
            colors[i * 4 + 0] = biomes[i].color.r / 255.0f;
            colors[i * 4 + 1] = biomes[i].color.g / 255.0f;
            colors[i * 4 + 2] = biomes[i].color.b / 255.0f;
            colors[i * 4 + 3] = 1.0f;
        }

        SetShaderValueV(biomeShader, GetShaderLocation(biomeShader, "uBiomeColor"), colors,
                        SHADER_UNIFORM_VEC4, biomeCount);
    }

    BuildIslands();

    islandShader = LoadShader(0, "resources/Island.fs");

    int islandsCount = islands.size();
    SetShaderValue(islandShader, GetShaderLocation(islandShader, "uIslandsCount"), &islandsCount,
                   SHADER_UNIFORM_INT);

    Vector2 islandStarts[512], islandEnds[512];
    for (size_t i = 0; i < islands.size(); i++)
    {
        islandStarts[i] = islands[i].p1;
        islandEnds[i] = islands[i].p2;
    }
    SetShaderValueV(islandShader, GetShaderLocation(islandShader, "uIslandStarts"),
                    (float*)&islandStarts, SHADER_UNIFORM_VEC2, islandsCount);
    SetShaderValueV(islandShader, GetShaderLocation(islandShader, "uIslandEnds"),
                    (float*)&islandEnds, SHADER_UNIFORM_VEC2, islandsCount);
}

void DrawStats(const Island& island, Color color)
{
    // Do not draw anything if the scale is too small
    float scale = 0.01f / perlinScale;
    if (scale < 0.05f) return;

    // Constants
    const float margin = 100 * scale, lockScale = 0.5f * scale, woodScale = 0.15f * scale,
                ironScale = 0.15f * scale, humanScale = 0.075f * scale, textScale = 120 * scale;

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
        DrawTextureEx(lockTexture, center + offset, 0, lockScale, color);
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
    DrawRectangleRounded({center.x + offset.x - lockTexture.width * lockScale / 2,
                          center.y + offset.y - margin / 2, lockTexture.width * lockScale * 2,
                          woodTexture.height * woodScale + ironTexture.height * ironScale +
                              humanTexture.height * humanScale + margin * 3},
                         0.25f, 16, {0, 0, 0, 127});

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
    DrawTextureEx(humanTexture, center + offset - Vector2{humanTexture.width * humanScale / 2, 0},
                  0, humanScale, WHITE);
    {
        Vector2 textOffset = GetTextOffset(humanTexture, humanScale);
        DrawText(std::to_string(island.peopleCount).c_str(), textOffset.x, textOffset.y, textScale,
                 WHITE);
    }
    offset.y += humanTexture.height * humanScale + margin;
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
    DrawTextureEx(humanTexture, offset + Vector2{humanTexture.width * humanScale / 2, 0}, 0, humanScale,
                  WHITE);
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

void DrawFrame()
{
    BeginDrawing();

    ClearBackground(BLACK);

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
#if !defined(PLATFORM_WEB)
    windowSize /= GetWindowScaleDPI();
#endif

    SetShaderValue(biomeShader, GetShaderLocation(biomeShader, "uOffset"), (float*)&perlinOffset,
                   SHADER_UNIFORM_VEC2);
    SetShaderValue(islandShader, GetShaderLocation(islandShader, "uOffset"), (float*)&perlinOffset,
                   SHADER_UNIFORM_VEC2);

    SetShaderValue(biomeShader, GetShaderLocation(biomeShader, "uMapSize"), (float*)&mapSize,
                   SHADER_UNIFORM_VEC2);

    Vector2 mousePosition = RaylibToGlsl(GetMousePosition());
    SetShaderValue(islandShader, GetShaderLocation(islandShader, "uMouse"), (float*)&mousePosition,
                   SHADER_UNIFORM_VEC2);

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
        DrawStats(islands[i], ColorFromHSV(i * 360.0f / islands.size(), 1, 1));
    }

    DrawResources();

    DrawUI();

    EndDrawing();

    double deltaTime = GetTime() - timer;

    if (IsKeyDown(KEY_UP)) perlinOffset.y += panSensitivity * perlinScale * deltaTime;
    if (IsKeyDown(KEY_DOWN)) perlinOffset.y -= panSensitivity * perlinScale * deltaTime;
    if (IsKeyDown(KEY_LEFT)) perlinOffset.x -= panSensitivity * perlinScale * deltaTime;
    if (IsKeyDown(KEY_RIGHT)) perlinOffset.x += panSensitivity * perlinScale * deltaTime;

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
            if (v.x >= islands[i].p1.x && v.x <= islands[i].p2.x && v.y >= islands[i].p1.y &&
                v.y <= islands[i].p2.y)
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

    timer = GetTime();

    if (lastVsync != vsync)
    {
        lastVsync = vsync;
        if (!vsync)
            ClearWindowState(FLAG_VSYNC_HINT);
        else
            SetWindowState(FLAG_VSYNC_HINT);
    }
}
