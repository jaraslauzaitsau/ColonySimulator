// SPDX-FileCopyrightText: 2025 SemkiShow
// SPDX-FileContributor: Jaraslau Zaitsau
//
// SPDX-License-Identifier: GPL-3.0-only

#include "Island.hpp"
#include "Drawing.hpp"
#include "Human.hpp"
#include "Languages.hpp"
#include "Pathfinding.hpp"
#include "Perlin.hpp"
#include "Settings.hpp"
#include "UI.hpp"
#include "Ship.hpp"
#include "Utils.hpp"
#include <cfloat>
#include <climits>
#include <cmath>
#include <iostream>
#include <raygui.h>
#include <raymath.h>
#include <unordered_map>

#define K_WOOD_COLONIZE 0.05f
#define K_IRON_COLONIZE 0.004f
#define K_WOOD 0.02f
#define K_WOOD_GROWTH 0.002f
#define K_IRON 0.005f
#define K_PEOPLE 0.001f
#define K_PEOPLE_GROWTH 0.001f
#define K_PEOPLE_MAX 0.1f

#define K_WOOD_GET 3
#define K_IRON_GET 1
#define K_EFFICIENCY 5

inline Color rgb(unsigned char r, unsigned char g, unsigned char b) { return {r, g, b, 255}; }

std::vector<Biome> biomes = {{-1, rgb(0, 0, 255)},     {-0.5, rgb(0, 136, 255)},
                             {0, rgb(97, 218, 255)},   {0.1, rgb(251, 254, 145)},
                             {0.2, rgb(33, 171, 42)},  {0.5, rgb(184, 184, 205)},
                             {0.6, rgb(255, 255, 255)}};
std::vector<Island> islands;

int woodTotal = 0, ironTotal = 0, peopleTotal = 0;

// Custom hash for std::pair<size_t, size_t>
// Required as C++20 is not supported
template <class T1, class T2> struct std::hash<std::pair<T1, T2>>
{
    size_t operator()(const std::pair<T1, T2>& p) const
    {
        size_t hash1 = std::hash<T1>{}(p.first);
        size_t hash2 = std::hash<T2>{}(p.second);
        return hash1 ^ hash2;
    }
};

std::unordered_map<std::pair<size_t, size_t>, Path> pathCache;

Vector2 Island::GetRandomPoint()
{
    Vector2 pos;
    do
    {
        pos.x = GetRandomFloat(p1.x, p2.x);
        pos.y = GetRandomFloat(p1.y, p2.y);
    } while (GetPerlin(pos) < LAND_START);
    return pos;
}

void Island::Colonize()
{
    if (colonized || colonizationInProgress || woodTotal < woodColonize || ironTotal < ironColonize)
        return;
    colonizationInProgress = true;
    woodTotal -= woodColonize;
    ironTotal -= ironColonize;
    SendPeople(1);
}

void Island::SendPeople(int count)
{
    if (peopleCount + count > peopleMax) return;
    int maxPeopleIslandId = (&islands[0] == this ? 1 : 0);
    for (size_t i = 0; i < islands.size(); i++)
    {
        if (&islands[i] == this) continue;
        if (islands[i].peopleCount > islands[maxPeopleIslandId].peopleCount) maxPeopleIslandId = i;
    }
    if (islands[maxPeopleIslandId].peopleCount < count) return;
    islands[maxPeopleIslandId].peopleCount -= count;
    ships.emplace_back(Ship(islands[maxPeopleIslandId].index, this->index, count));

    int counter = 0;
    for (size_t i = 0; i < people.size(); i++)
    {
        if (counter >= count) break;
        if (people[i].islandIdx != maxPeopleIslandId) continue;
        people.erase(people.begin() + i);
        counter++;
    }
}

void Island::AddPeople(int count)
{
    if (!colonized) colonized = true;
    peopleCount += count;
    for (int i = 0; i < count; i++)
    {
        people.emplace_back(GetRandomPoint(), index);
    }
}

void Island::GrowthTick()
{
    if (!colonized) return;
    woodCount += woodGrowth;
    woodCount = fmin(woodCount, woodMax);
    if (peopleCount >= 2)
    {
        addPeopleFraction += peopleGrowth * sqrt(peopleCount) * efficiency / 100;

        int delta = addPeopleFraction;
        delta = fmin(delta, peopleMax - peopleCount);
        addPeopleFraction -= delta;
        peopleCount += delta;
        peopleTotal += delta;
        for (int i = 0; i < delta; i++)
        {
            people.emplace_back(GetRandomPoint(), index);
        }
    }
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

void Island::DrawStats()
{
    // Do not draw anything if the scale is too small
    float scale = 0.01f / perlinScale;
    if (scale < 0.05f) return;

    // Constants
    const float margin = 100 * scale, lockScale = 0.5f * scale, woodScale = 0.15f * scale,
                ironScale = 0.15f * scale, humanScale = 0.075f * scale, textScale = 175 * scale,
                buttonScale = 300 * scale;

    // Calculate the center point of the island
    Vector2 center = GlslToRaylib((p2 + p1) / 2);
    center.x -= lockTexture.width * scale / 2;
    center.y -= lockTexture.height * scale / 2;

    // Calculate the offset
    Vector2 offset = {0, -lockTexture.height * lockScale / 2};
    if (colonized) offset *= -1;

    // Draw lock
    if (!colonized)
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
                         (colonized ? humanTexture.height * humanScale + margin : 0)};
    DrawRectangleRounded(rec, 0.25f, 16, {0, 0, 0, 127});
    if (!colonized) DrawRectangleRoundedLinesEx(rec, 0.25f, 16, 3, RED);

    // Draw wood
    DrawTextureEx(woodTexture, center + offset - Vector2{woodTexture.width * woodScale / 2, 0}, 0,
                  woodScale, WHITE);
    {
        Vector2 textOffset = GetTextOffset(woodTexture, woodScale);
        DrawTextCustom(std::to_string(colonized ? woodCount : woodColonize).c_str(), textOffset,
                       textScale, WHITE);
    }
    offset.y += woodTexture.height * woodScale + margin;

    // Draw iron
    DrawTextureEx(ironTexture, center + offset - Vector2{ironTexture.width * ironScale / 2, 0}, 0,
                  ironScale, WHITE);
    {
        Vector2 textOffset = GetTextOffset(ironTexture, ironScale);
        DrawTextCustom(std::to_string(colonized ? ironCount : ironColonize).c_str(), textOffset,
                       textScale, WHITE);
    }
    offset.y += ironTexture.height * ironScale + margin;

    // Draw people
    if (colonized)
    {
        DrawTextureEx(humanTexture,
                      center + offset - Vector2{humanTexture.width * humanScale / 2, 0}, 0,
                      humanScale, WHITE);
        {
            Vector2 textOffset = GetTextOffset(humanTexture, humanScale);
            DrawTextCustom(std::to_string(peopleCount).c_str(), textOffset, textScale, WHITE);
        }
        offset.y += humanTexture.height * humanScale + margin;
    }

    // Draw taxes button
    if (colonized)
    {
        auto buttonRec = rec;
        buttonRec.width = buttonRec.height = buttonScale;
        buttonRec.x += rec.width;
        if (GuiButton(buttonRec, "#142#")) islandEditIdx = index;
    }
}

Json Island::ToJSON()
{
    Json json;

    json["p1"].format = JsonFormat::Inline;
    json["p1"].push_back(p1.x);
    json["p1"].push_back(p1.y);

    json["p2"].format = JsonFormat::Inline;
    json["p2"].push_back(p2.x);
    json["p2"].push_back(p2.y);

    json["area"] = area;
    json["woodColonize"] = woodColonize;
    json["ironColonize"] = ironColonize;
    json["woodCount"] = woodCount;
    json["woodGrowth"] = woodGrowth;
    json["woodMax"] = woodMax;
    json["ironCount"] = ironCount;
    json["peopleCount"] = peopleCount;
    json["peopleMax"] = peopleMax;
    json["peopleGrowth"] = peopleGrowth;
    json["addPeopleFraction"] = addPeopleFraction;
    json["colonized"] = colonized;
    json["taxes"] = taxes;
    json["efficiency"] = efficiency;

    return json;
}

Island Island::LoadJSON(Json& json)
{
    Island island;
    island.p1 = {static_cast<float>(json["p1"][0].GetDouble()),
                 static_cast<float>(json["p1"][1].GetDouble())};
    island.p2 = {static_cast<float>(json["p2"][0].GetDouble()),
                 static_cast<float>(json["p2"][1].GetDouble())};
    island.area = json["area"].GetDouble();
    island.woodColonize = json["woodColonize"].GetInt();
    island.ironColonize = json["ironColonize"].GetInt();
    island.woodCount = json["woodCount"].GetInt();
    island.woodGrowth = json["woodGrowth"].GetInt();
    island.woodMax = json["woodMax"].GetInt();
    island.ironCount = json["ironCount"].GetInt();
    island.peopleCount = json["peopleCount"].GetInt();
    island.peopleMax = json["peopleMax"].GetInt();
    island.peopleGrowth = json["peopleGrowth"].GetDouble();
    island.addPeopleFraction = json["addPeopleFraction"].GetDouble();
    island.colonized = json["colonized"].GetBool();
    island.taxes = json["taxes"].GetInt();
    island.efficiency = json["efficiency"].GetInt();
    return island;
}

void BuildIslands(float& loadingPercent, std::atomic<bool>& finished, float stepSize)
{
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
        loadingPercent += 1.0f / maxY / 2 * 100;
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
        loadingPercent += 1.0f / maxY / 2 * 100;
    }

    // Add large enough islands to the main vector
    int minIslandArea = 125 / stepSize / stepSize;
    int passed = 0;
    islands.clear();
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
                             cost * K_IRON, area * K_PEOPLE_GROWTH, area * K_PEOPLE_MAX);
        islands.back().index = islands.size() - 1;
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
    peopleTotal = startIsland.area * K_PEOPLE;
    peopleTotal = fmax(2, peopleTotal);
    startIsland.peopleCount = peopleTotal;
    for (int i = 0; i < startIsland.peopleCount; i++)
    {
        people.emplace_back(startIsland.GetRandomPoint(), minDistanceIslandIdx);
    }

    // Prevent softlocking by having enough people to extract iron and enough iron to colonize
    startIsland.peopleMax = fmax(3, startIsland.peopleMax);
    startIsland.ironCount *= 3;

    finished = true;
}

void BuildMap()
{
    auto func = [](std::string& label, float& loadingPercent, std::atomic<bool>& finished)
    {
        label = labels["Loading map..."];
        woodTotal = ironTotal = peopleTotal = 0;
        BuildIslands(loadingPercent, finished, 0.1f);
    };
    ShowLoadingScreen(true, func);
    ReloadIslandShaderValues();
}
