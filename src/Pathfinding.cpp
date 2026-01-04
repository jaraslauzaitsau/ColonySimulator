// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "Island.hpp"
#include "Pathfinding.hpp"
#include "Perlin.hpp"
#include "Settings.hpp"
#include <algorithm>
#include <map>
#include <queue>
#include <raymath.h>

std::vector<std::vector<Path>> pathCache;

bool operator<(const Vector2& a, const Vector2& b)
{
    const float eps = 0.01f;
    if (std::abs(a.x - b.x) > eps) return a.x < b.x;
    if (std::abs(a.y - b.y) > eps) return a.y < b.y;
    return false;
}

struct Node
{
    Vector2 pos;
    float cost;

    bool operator>(const Node& other) const { return cost > other.cost; }
};

float GetHeuristic(Vector2 a, Vector2 b) { return std::abs(a.x - b.x) + std::abs(a.y - b.y); }

Path FindPath(Vector2 start, Vector2 goal, bool onLand, float stepSize)
{
    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> openSet;
    std::map<Vector2, Vector2> cameFrom;
    std::map<Vector2, float> cost; // Cost from start to current node

    openSet.push({start, GetHeuristic(start, goal)});
    cost[start] = 0.0f;

    while (!openSet.empty())
    {
        Vector2 current = openSet.top().pos;
        openSet.pop();

        float dx = current.x - goal.x;
        float dy = current.y - goal.y;

        // If we are within half a step of the goal, we've arrived
        if ((dx * dx + dy * dy) < (stepSize * stepSize * 0.5f))
        {
            // Reconstruct path
            Path path;
            while (cameFrom.count(current))
            {
                path.push_back(current);
                current = cameFrom[current];
            }
            std::reverse(path.begin(), path.end());
            return path;
        }

        Vector2 neighbors[4] = {{0, 1}, {0, -1}, {1, 0}, {-1, 0}};
        for (auto& offset: neighbors)
        {
            Vector2 neighbor = {current.x + offset.x * stepSize, current.y + offset.y * stepSize};

            if (neighbor.x < -mapSize.x / 2 || neighbor.x > mapSize.x / 2 ||
                neighbor.y < -mapSize.y / 2 || neighbor.y > mapSize.y / 2)
                continue;

            auto neighborPerlin = GetPerlin(neighbor);
            if (onLand && neighborPerlin < LAND_START) continue;
            if (!onLand && neighborPerlin >= LAND_START) continue;

            // Weight calculation
            float currentNoise = GetPerlin(current);
            float neighborNoise = neighborPerlin;

            // Penalty for climbing steep noise gradients
            float diff = std::abs(neighborNoise - currentNoise);
            float moveCost = 1.0f + std::pow(diff * 10.0f, 2);

            float tentativeCost = cost[current] + moveCost;

            if (!cost.count(neighbor) || tentativeCost < cost[neighbor])
            {
                cameFrom[neighbor] = current;
                cost[neighbor] = tentativeCost;
                float cost = tentativeCost + GetHeuristic(neighbor, goal);
                openSet.push({neighbor, cost});
            }
        }
    }
    return {};
}
