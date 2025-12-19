// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "Island.hpp"
#include "Perlin.hpp"
#include "Settings.hpp"
#include <cfloat>
#include <climits>
#include <iostream>
#include <unordered_map>

inline Color rgb(unsigned char r, unsigned char g, unsigned char b) { return {r, g, b, 255}; }

std::vector<Biome> biomes = {{-1, rgb(0, 0, 255)},     {-0.5, rgb(0, 136, 255)},
                             {0, rgb(97, 218, 255)},   {0.1, rgb(251, 254, 145)},
                             {0.2, rgb(33, 171, 42)},  {0.5, rgb(184, 184, 205)},
                             {0.6, rgb(255, 255, 255)}};
std::vector<Island> islands;
std::vector<std::pair<Vector2, Color>> points;

int woodTotal = 0, ironTotal = 0, peopleTotal = 0;

void Island::Colonize()
{
    if (colonized || woodTotal < woodColonize || ironTotal < ironColonize) return;
    colonized = true;
    woodTotal -= woodColonize;
    ironTotal -= ironColonize;
    SendPeople(1);
}

void Island::SendPeople(int count)
{
    int maxPeopleIslandId = (&islands[0] == this ? 1 : 0);
    for (size_t i = 0; i < islands.size(); i++)
    {
        if (&islands[i] == this) continue;
        if (islands[i].peopleCount > islands[maxPeopleIslandId].peopleCount) maxPeopleIslandId = i;
    }
    if (islands[maxPeopleIslandId].peopleCount < count) return;
    islands[maxPeopleIslandId].peopleCount -= count;
    peopleCount += count;
}

void Island::GrowthTick()
{
    if (!colonized) return;
    woodCount += woodGrowth;
    woodCount = fmin(woodCount, woodMax);
    {
        int delta = fmin(woodCount, K_WOOD_GET * peopleCount * taxes / 100 * efficiency / 100);
        woodCount -= delta;
        woodTotal += delta;
    }
    {
        int delta = fmin(ironCount, K_IRON_GET * peopleCount * taxes / 100 * efficiency / 100);
        ironCount -= delta;
        ironTotal += delta;
    }
    if (taxes < DEFAULT_TAXES)
    {
        efficiency += rand() % (DEFAULT_TAXES - taxes) / K_EFFICIENCY;
        efficiency = fmin(100, efficiency);
    }
    if (taxes > DEFAULT_TAXES)
    {
        efficiency -= rand() % (taxes - DEFAULT_TAXES) / K_EFFICIENCY;
        efficiency = fmax(0, efficiency);
    }
}

#define LAND_START biomes[3].startLevel

void BuildIslands(float stepSize)
{
    islands.clear();

    // Find islands
    size_t maxX = ceil(mapSize.x / stepSize) + 1, maxY = ceil(mapSize.y / stepSize) + 1;
    std::vector<std::vector<int>> map(maxY, std::vector<int>(maxX, INT_MAX));
    std::unordered_map<int, int> same;
    size_t counter = 0;
    for (size_t i = 0; i < maxY; i++)
    {
        for (size_t j = 0; j < maxX; j++)
        {
            if (GetPerlin({j * stepSize - mapSize.x / 2, i * stepSize - mapSize.y / 2}) <
                LAND_START)
                continue;
            if (j > 0) map[i][j] = fmin(map[i][j], map[i][j - 1]);
            if (i > 0) map[i][j] = fmin(map[i][j], map[i - 1][j]);
            if (j > 0 && i > 0)
            {
                auto n1 = map[i][j - 1], n2 = map[i - 1][j];
                if (n1 > n2) std::swap(n1, n2);
                if (n1 != n2 && same.find(n2) == same.end()) same[n2] = n1;
            }
            if (map[i][j] == INT_MAX) map[i][j] = counter++;
        }
    }
    std::cout << "Total island count: " << counter - same.size() << '\n';

    // Calculate islands' areas
    std::vector<int> islandAreas(counter, 0);
    std::vector<std::pair<Vector2, Vector2>> islandCorners(
        counter, {{FLT_MAX, FLT_MAX}, {-FLT_MAX, -FLT_MAX}});
    for (size_t i = 0; i < maxY; i++)
    {
        for (size_t j = 0; j < maxX; j++)
        {
            if (map[i][j] == INT_MAX) continue;

            int idx = map[i][j];
            while (same.find(idx) != same.end())
                idx = same[idx];
            map[i][j] = idx;
            islandAreas[idx]++;

            auto& corner = islandCorners[idx];
            corner.first.x = fmin(corner.first.x, j * stepSize - mapSize.x / 2);
            corner.first.y = fmin(corner.first.y, i * stepSize - mapSize.y / 2);
            corner.second.x = fmax(corner.second.x, j * stepSize - mapSize.x / 2);
            corner.second.y = fmax(corner.second.y, i * stepSize - mapSize.y / 2);
        }
    }

    // Add large enough islands to the main vector
    int minIslandArea = 125 / stepSize / stepSize;
    int passed = 0;
    for (size_t i = 0; i < counter; i++)
    {
        if (islandAreas[i] < minIslandArea) continue;

        auto& corner = islandCorners[i];
        Vector2 center = {(corner.second.x + corner.first.x) / 2,
                          (corner.second.y + corner.first.y) / 2};
        float distance = Vector2Distance(center, {0, 0});
        float area = islandAreas[i] * stepSize * stepSize;
        float cost = distance * area;
        islands.emplace_back(corner.first, corner.second, area, cost * K_WOOD_COLONIZE,
                             cost * K_IRON_COLONIZE, cost * K_WOOD, cost * K_WOOD_GROWTH,
                             cost * K_IRON);
        passed++;
    }
    std::cout << "Found " << passed << " large enough islands\n";

    // Set the closest island to center as colonized
    int minDistanceIslandIdx = 0;
    for (size_t i = 0; i < islands.size(); i++)
    {
        const auto &island = islands[i], &minIsland = islands[minDistanceIslandIdx];
        if (abs(Vector2Distance(Vector2{0, 0}, (island.p1 + island.p2) / 2)) <
            abs(Vector2Distance(Vector2{0, 0}, (minIsland.p1 + minIsland.p2) / 2)))
            minDistanceIslandIdx = i;
    }
    islands[minDistanceIslandIdx].colonized = true;

    // Set start resources
    auto& startIsland = islands[minDistanceIslandIdx];
    float distance = Vector2Distance(Vector2{0, 0}, (startIsland.p1 + startIsland.p2) / 2);
    float cost = distance * startIsland.area;
    peopleTotal = cost * K_PEOPLE;
    peopleTotal = fmax(2, peopleTotal);
    startIsland.peopleCount = peopleTotal;
    startIsland.ironCount *= 3;
}
