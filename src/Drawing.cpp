// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "Drawing.hpp"
#include "Settings.hpp"
#include "UI.hpp"
#include <algorithm>
#include <ctime>
#include <string>
#include <unordered_map>
#include <vector>

std::unordered_map<char, int> hexMap{
    {'0', 0}, {'1', 1}, {'2', 2},  {'3', 3},  {'4', 4},  {'5', 5},  {'6', 6},  {'7', 7},
    {'8', 8}, {'9', 9}, {'a', 10}, {'b', 11}, {'c', 12}, {'d', 13}, {'e', 14}, {'f', 15}};
inline Color ColorFromHex(const std::string& hexIn)
{
    std::string hex = hexIn;
    std::transform(hexIn.begin(), hexIn.end(), hex.begin(), ::tolower);
    unsigned char r = 16 * hexMap[hex[1]] + hexMap[hex[2]],
                  g = 16 * hexMap[hex[3]] + hexMap[hex[4]],
                  b = 16 * hexMap[hex[5]] + hexMap[hex[6]];
    return {r, g, b, 255};
}

struct Biome
{
    float startLevel;
    Color color;
    Biome(double startLevel, const Color& color) : startLevel(startLevel), color(color) {}
};

std::vector<Biome> biomes = {{-1, ColorFromHex("#0000ff")},    {-0.7, ColorFromHex("#0088ffff")},
                             {0, ColorFromHex("#61daffff")},   {0.1, ColorFromHex("#fbfe91ff")},
                             {0.2, ColorFromHex("#21ab2aff")}, {0.5, ColorFromHex("#b8b8cdff")},
                             {0.6, ColorFromHex("#ffffffff")}};

Vector2 windowSize{16 * 50 * 2, 9 * 50 * 2};
Shader biomeShader;
bool lastVsync = vsync;

void InitGPU()
{
    biomeShader = LoadShader(0, "resources/Perlin.fs");

    int biomeCount = (int)biomes.size();
    SetShaderValue(biomeShader, GetShaderLocation(biomeShader, "uBiomeCount"), &biomeCount,
                   SHADER_UNIFORM_INT);

    float seed = GetTime();
    SetShaderValue(biomeShader, GetShaderLocation(biomeShader, "uSeed"), &seed, SHADER_UNIFORM_FLOAT);

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
}

void DrawFrame()
{
    ClearBackground(BLACK);

    float scale = perlinScale;
    SetShaderValue(biomeShader, GetShaderLocation(biomeShader, "uScale"), &scale,
                   SHADER_UNIFORM_FLOAT);

    windowSize = {GetRenderWidth() * 1.0f, GetRenderHeight() * 1.0f};
    float res[2] = {windowSize.x, windowSize.y};
    SetShaderValue(biomeShader, GetShaderLocation(biomeShader, "uResolution"), res,
                   SHADER_UNIFORM_VEC2);

    BeginShaderMode(biomeShader);
    DrawRectangle(0, 0, windowSize.x, windowSize.y, WHITE);
    EndShaderMode();

    DrawUI();

    if (lastVsync != vsync)
    {
        lastVsync = vsync;
        if (!vsync)
            ClearWindowState(FLAG_VSYNC_HINT);
        else
            SetWindowState(FLAG_VSYNC_HINT);
    }
}
