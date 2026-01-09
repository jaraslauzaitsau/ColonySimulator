// SPDX-FileCopyrightText: 2025 SemkiShow
// SPDX-FileContributor: Jaraslau Zaitsau
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
#include <raylib.h>
#include <raymath.h>

bool shouldClose = false;

Menu currentMenu = Menu::Main;

Vector2 windowSize{16 * 50, 9 * 50};
double timer = 0;
bool lastVsync = vsync;

Shader perlinShader;

Texture lockTexture;
Texture woodTexture;
Texture ironTexture;
Texture humanTexture;
Texture shipTexture;

Font myFont;

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

void InitGPU()
{
    lockTexture = LoadTexture("resources/textures/lock.png");
    woodTexture = LoadTexture("resources/textures/wooden_log.png");
    ironTexture = LoadTexture("resources/textures/iron_ingot.png");
    humanTexture = LoadTexture("resources/textures/human.png");
    shipTexture = LoadTexture("resources/textures/ship.png");

    const char* symbols =
        "`1234567890-=qwertyuiop[]\\asdfghjkl;'zxcvbnm,./~!@#$%^&*()_+QWERTYUIOP{}|ASDFGHJKL:\"ZXCVBNM<>?ęóąśłżźćńĘÓĄŚŁŻŹĆŃ";
    int codepointCount = 0;
    int* codepoints = LoadCodepoints(symbols, &codepointCount);
    myFont = LoadFontEx("resources/fonts/JetBrainsMono-Bold.ttf", 512, codepoints, codepointCount);

    perlinShader = LoadShader(0, "resources/shaders/Perlin.fs");

    int biomeCount = (int)biomes.size();
    SetShaderValue(perlinShader, GetShaderLocation(perlinShader, "uBiomeCount"), &biomeCount,
                   SHADER_UNIFORM_INT);

    {
        float starts[8];
        for (size_t i = 0; i < biomes.size(); i++)
            starts[i] = biomes[i].startLevel;

        SetShaderValueV(perlinShader, GetShaderLocation(perlinShader, "uBiomeStart"), starts,
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

        SetShaderValueV(perlinShader, GetShaderLocation(perlinShader, "uBiomeColor"), colors,
                        SHADER_UNIFORM_VEC4, biomeCount);
    }
}

void DrawFrame()
{
    if (IsWindowMinimized())
    {
        if (currentMenu == Menu::Game) currentMenu = Menu::Pause;

        PollInputEvents();
        WaitTime(0.1);
        return;
    }

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

    if (currentMenu == Menu::Game) ProcessPlayerInput(GetFrameTime());

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
    UnloadShader(perlinShader);

    UnloadTexture(lockTexture);
    UnloadTexture(woodTexture);
    UnloadTexture(ironTexture);
    UnloadTexture(humanTexture);
    UnloadTexture(shipTexture);

    UnloadFont(myFont);
}
