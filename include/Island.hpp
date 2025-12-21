// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <atomic>
#include <raylib.h>
#include <vector>

struct Biome
{
    float startLevel;
    Color color;
    Biome(float startLevel, const Color& color) : startLevel(startLevel), color(color) {}
};

#define K_WOOD_COLONIZE 0.05f
#define K_IRON_COLONIZE 0.004f
#define K_WOOD 0.02f
#define K_WOOD_GROWTH 0.002f
#define K_IRON 0.005f
#define K_PEOPLE 0.001f
#define K_PEOPLE_GROWTH 0.00003f
#define K_PEOPLE_MAX 0.005f

#define GROWTH_PERIOD 1
#define K_WOOD_GET 3
#define K_IRON_GET 1
#define K_EFFICIENCY 5
#define DEFAULT_TAXES 67

struct Island
{
    Vector2 p1 = {0, 0}, p2 = {0, 0};
    float area = 0;
    int woodColonize = 0, ironColonize = 0,
        woodCount = 0, woodGrowth = 0, woodMax = 0, ironCount = 0,
        peopleCount = 0, peopleMax = 0;
    float peopleGrowth = 0, addPeopleFraction = 0;
    bool colonized = false;
    int taxes = DEFAULT_TAXES, efficiency = 50;

    Island(Vector2 p1, Vector2 p2, float area, int woodColonize, int ironColonize, int woodCount,
           int woodGrowth, int ironCount, float peopleGrowth, int peopleMax)
        : p1(p1), p2(p2), area(area), woodColonize(woodColonize), ironColonize(ironColonize),
          woodCount(woodCount), woodGrowth(woodGrowth), ironCount(ironCount),
          peopleMax(peopleMax), peopleGrowth(peopleGrowth)
    {
        woodMax = woodCount;
    }

    void Colonize();
    void SendPeople(int count);
    void GrowthTick();
};

extern std::vector<Biome> biomes;
extern std::vector<Island> islands;
extern std::vector<std::pair<Vector2, Color>> points;

extern int woodTotal;
extern int ironTotal;
extern int peopleTotal;

void BuildIslands(std::atomic<bool>& finished, float stepSize = 0.1f);
void LoadMap();
