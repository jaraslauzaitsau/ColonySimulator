// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "Drawing.hpp"
#include "Drawing/GameMenu.hpp"
#include "Drawing/MainMenu.hpp"
#include "Drawing/PauseMenu.hpp"
#include "Island.hpp"
#include "Perlin.hpp"
#include "Settings.hpp"
#include <ctime>
#include <raygui.h>

Menu currentMenu = Menu::Main;

Vector2 windowSize{16 * 50, 9 * 50};
double timer = 0;
bool lastVsync = vsync;

Shader biomeShader;
Shader islandShader;

Texture lockTexture;
Texture woodTexture;
Texture ironTexture;
Texture humanTexture;

Vector2 RaylibToGlsl(Vector2 v)
{
    v.y = windowSize.y - v.y;
    v -= windowSize / 2;
    v *= GetWindowScaleDPI();
    v *= perlinScale;
    v += perlinOffset;
    return v;
}

Vector2 GlslToRaylib(Vector2 v)
{
    v.y *= -1;
    v /= GetWindowScaleDPI() * perlinScale;
    v += windowSize / 2;
    v.x -= perlinOffset.x / perlinScale / GetWindowScaleDPI().x;
    v.y += perlinOffset.y / perlinScale / GetWindowScaleDPI().y;
    return v;
}

void UpdateWindowSize()
{
    windowSize = {(float)GetRenderWidth(), (float)GetRenderHeight()};
    windowSize /= GetWindowScaleDPI();
}

void ReloadIslandShaderValues()
{
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
    islandShader = LoadShader(0, "resources/Island.fs");
}

void DrawFrame()
{
    BeginDrawing();

    ClearBackground(BLACK);

    UpdateWindowSize();

    switch (currentMenu)
    {
    case Menu::Main:
        DrawMainMenu();
        break;
    case Menu::Game:
        DrawGameMenu();
        break;
    case Menu::Pause:
        DrawGameMenu();
        DrawPauseMenu();
        break;
    default:
        break;
    }

    EndDrawing();

    double deltaTime = GetTime() - timer;

    if (currentMenu == Menu::Game) ProcessPlayerInput(deltaTime);

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

void FreeResources()
{
    UnloadShader(biomeShader);
    UnloadShader(islandShader);

    UnloadTexture(lockTexture);
    UnloadTexture(woodTexture);
    UnloadTexture(ironTexture);
    UnloadTexture(humanTexture);
}
