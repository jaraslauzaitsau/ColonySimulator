// SPDX-FileCopyrightText: 2025 SemkiShow
// SPDX-FileContributor: Jaraslau Zaitsau
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "JSON.hpp"
#include <atomic>
#include <raylib.h>
#include <vector>

struct Biome
{
    float startLevel;
    Color color;
    Biome(float startLevel, const Color& color) : startLevel(startLevel), color(color) {}
};

#define GROWTH_PERIOD 1
#define DEFAULT_TAXES 67

struct Island
{
    Vector2 p1 = {0, 0}, p2 = {0, 0};
    float area = 0;
    int woodColonize = 0, ironColonize = 0, woodCount = 0, woodGrowth = 0, woodMax = 0,
        ironCount = 0, peopleCount = 0, peopleMax = 0;
    float peopleGrowth = 0, addPeopleFraction = 0;
    bool colonized = false;
    int taxes = DEFAULT_TAXES, efficiency = 50;
    int index = -1;

    Island() = default;
    Island(Vector2 p1, Vector2 p2, float area, int woodColonize, int ironColonize, int woodCount,
           int woodGrowth, int ironCount, float peopleGrowth, int peopleMax)
        : p1(p1), p2(p2), area(area), woodColonize(woodColonize), ironColonize(ironColonize),
          woodCount(woodCount), woodGrowth(woodGrowth), ironCount(ironCount), peopleMax(peopleMax),
          peopleGrowth(peopleGrowth)
    {
        woodMax = woodCount;
    }

    Vector2 GetRandomPoint();
    void Colonize();
    void SendPeople(int count);
    void GrowthTick();

    JSON ToJSON();
    static Island LoadJSON(JSON& json);
};

extern std::vector<Biome> biomes;
extern std::vector<Island> islands;

#define LAND_START biomes[3].startLevel

extern int woodTotal;
extern int ironTotal;
extern int peopleTotal;

void BuildIslands(std::atomic<bool>& finished, float stepSize = 0.1f);
void BuildMap();
