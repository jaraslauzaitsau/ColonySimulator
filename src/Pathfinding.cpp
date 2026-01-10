// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "Pathfinding.hpp"
#include "Island.hpp"
#include "Perlin.hpp"
#include "Settings.hpp"
#include <algorithm>
#include <queue>
#include <raymath.h>

std::vector<ParentMap> pathMap;

std::vector<Vector2> directions{{0, -1}, {1, -1}, {1, 0},  {1, 1},
                                {0, 1},  {-1, 1}, {-1, 0}, {-1, -1}};

int Vector2ToInt(Vector2 v)
{
    int ix = (int)roundf(v.x + mapSize.x / 2.0f);
    int iy = (int)roundf(v.y + mapSize.y / 2.0f);

    ix = std::clamp(ix, 0, (int)mapSize.x - 1);
    iy = std::clamp(iy, 0, (int)mapSize.y - 1);

    return iy * (int)mapSize.x + ix;
}

Vector2 IntToVector2(int val)
{
    int ix = val % (int)mapSize.x;
    int iy = val / (int)mapSize.x;

    return {(float)ix - mapSize.x / 2.0f, (float)iy - mapSize.y / 2.0f};
}

bool IsInsideMap(Vector2 v)
{
    return v.x >= -mapSize.x / 2 && v.x < mapSize.x / 2 && v.y >= -mapSize.y / 2 &&
           v.y < mapSize.y / 2;
}

struct Node
{
    int idx;
    float cost;
    bool operator>(const Node& other) const { return cost > other.cost; }
};

void GeneratePathMap()
{
    pathMap.clear();
    pathMap.resize(islands.size());

    std::vector<bool> onLand(mapSize.x * mapSize.y, false);
    for (size_t i = 0; i < mapSize.x; i++)
    {
        for (size_t j = 0; j < mapSize.y; j++)
        {
            onLand[j * mapSize.x + i] = GetPerlin(IntToVector2(j * mapSize.x + i)) >= LAND_START;
        }
    }

    for (size_t i = 0; i < islands.size(); i++)
    {
        auto& island = islands[i];
        pathMap[i].assign(mapSize.x * mapSize.y, -1);

        std::priority_queue<Node, std::vector<Node>, std::greater<Node>> pq;
        std::vector<float> minCosts(mapSize.x * mapSize.y, std::numeric_limits<float>::max());

        Vector2 startPos = island.GetRandomPoint();
        Vector2 endPos = {0, 0};
        Vector2 dir = Vector2Normalize(endPos - startPos);
        do
        {
            startPos += dir;
        } while (onLand[Vector2ToInt(startPos)]);

        auto start = Vector2ToInt(startPos);
        minCosts[start] = 0;
        pq.push({start, 0});

        while (!pq.empty())
        {
            Node u = pq.top();
            pq.pop();

            if (u.cost > minCosts[u.idx]) continue;

            Vector2 uVec = IntToVector2(u.idx);

            for (auto& dir: directions)
            {
                Vector2 vVec = uVec + dir;
                if (!IsInsideMap(uVec + dir) || onLand[Vector2ToInt(uVec + dir)]) continue;

                int v = Vector2ToInt(vVec);

                float moveStep = (dir.x != 0 && dir.y != 0) ? 1.414f : 1.0f;
                float newCost = u.cost + moveStep;

                if (newCost < minCosts[v])
                {
                    minCosts[v] = newCost;
                    pathMap[i][v] = u.idx;
                    pq.push({v, newCost});
                }
            }
        }
    }
}

Path GetPath(Vector2 startPos, int targetIslandIdx)
{
    Path path;
    int parent = Vector2ToInt(startPos);
    while (parent != -1)
    {
        path.push_back(IntToVector2(parent));
        parent = pathMap[targetIslandIdx][parent];
    }
    return path;
}
