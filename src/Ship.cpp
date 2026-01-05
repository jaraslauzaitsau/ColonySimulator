// SPDX-FileCopyrightText: 2025 Jaraslau Zaitsau
// SPDX-FileContributor: SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "Ship.hpp"
#include "Island.hpp"
#include "Pathfinding.hpp"
#include "Perlin.hpp"
#include <raymath.h>
#include <vector>

std::vector<Ship> ships;

Ship::Ship(size_t sourceIndex, size_t targetIndex, int peopleCount)
    : sourceIndex(sourceIndex), targetIndex(targetIndex), people(peopleCount)
{
    // Find water nearest to the other island
    pos = islands[sourceIndex].GetRandomPoint();
    Vector2 end = islands[targetIndex].GetRandomPoint();
    Vector2 increment = Vector2Normalize(end - pos) * 0.25f;

    while (GetPerlin(pos) >= LAND_START)
        pos += increment;
    while (GetPerlin(end) >= LAND_START)
        end -= increment;

    // if (pathCache[sourceIndex][targetIndex].size() > 0)
    // {
    //     path = pathCache[sourceIndex][targetIndex];
    // }
    // else
    // {
    //     path = FindPath(pos, end, false, 1.0f);
    //     pathCache[sourceIndex][targetIndex] = path;
    // }
    // Leave this for now, as path cache needs to be loaded

    path = FindPath(pos, end, false, 1.0f);
    nextPointDir = Vector2Normalize(path[0] - pos);
}

void Ship::Move(float deltaTime)
{
    if (reached) return;
    Vector2 nextPos = pos + nextPointDir * SHIP_SPEED * deltaTime;
    if (Vector2Distance(pos, path[nextPointIdx]) > Vector2Distance(nextPos, path[nextPointIdx]))
    {
        pos = nextPos;
    }
    else
    {
        nextPointIdx++;
        if (nextPointIdx >= path.size())
        {
            islands[targetIndex].AddPeople(people);
            reached = true;
            return;
        }
        nextPointDir = Vector2Normalize(path[nextPointIdx] - pos);
        rotation = atan2f(nextPointDir.y, nextPointDir.x) * 180.0f / PI;
    }
}
