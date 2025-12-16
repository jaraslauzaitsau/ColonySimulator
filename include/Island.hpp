// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <raylib.h>
#include <vector>

struct Biome
{
    float startLevel;
    Color color;
    Biome(float startLevel, const Color& color) : startLevel(startLevel), color(color) {}
};

#define WOOD_COLONIZE_K 0.02f
#define IRON_COLONIZE_K 0.005f
#define WOOD_K 0.02f
#define WOOD_GROWTH_K 0.005f
#define IRON_K 0.005f
#define PEOPLE_K 0.005f

struct Island
{
    Vector2 p1 = {0, 0}, p2 = {0, 0};
    float area = 0;
    int woodColonize = 0, ironColonize = 0, woodCount = 0, woodGrowth = 0, ironCount = 0;
    bool colonized = false;

    Island(Vector2 p1, Vector2 p2, float area, int woodColonize, int ironColonize, int woodCount,
           int woodGrowth, int ironCount)
        : p1(p1), p2(p2), area(area), woodColonize(woodColonize), ironColonize(ironColonize),
          woodCount(woodCount), woodGrowth(woodGrowth), ironCount(ironCount)
    {
    }
};

extern std::vector<Biome> biomes;
extern std::vector<Island> islands;
extern std::vector<std::pair<Vector2, Color>> points;

extern int woodTotal;
extern int ironTotal;
extern int peopleTotal;

void BuildIslands(float stepSize = 0.1f);
