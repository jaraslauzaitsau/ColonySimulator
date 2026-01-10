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

Ship::Ship(int sourceIndex, int targetIndex, int peopleCount)
    : sourceIndex(sourceIndex), targetIndex(targetIndex), people(peopleCount)
{
    Vector2 startPos = islands[sourceIndex].GetRandomPoint();
    Vector2 endPos = islands[targetIndex].GetRandomPoint();
    Vector2 dir = Vector2Normalize(endPos - startPos);
    do
    {
        startPos += dir;
    } while (GetPerlin(startPos) >= LAND_START);

    path = GetPath(startPos, targetIndex);
    pos = path[0];

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
            reached = true;
            return;
        }

        nextPointDir = Vector2Normalize(path[nextPointIdx] - pos);
        if (nextPointDir.x > 0.1) flip = 1;
        if (nextPointDir.x < -0.1) flip = -1;
    }
}

Json Ship::ToJSON()
{
    Json json;
    json["sourceIndex"] = sourceIndex;
    json["targetIndex"] = targetIndex;

    json["pos"].format = JsonFormat::Inline;
    json["pos"].push_back(pos.x);
    json["pos"].push_back(pos.y);

    json["people"] = people;
    return json;
}

Ship Ship::LoadJSON(Json& json)
{
    Ship ship;
    ship.sourceIndex = json["sourceIndex"].GetInt();
    ship.targetIndex = json["targetIndex"].GetInt();
    ship.people = json["people"].GetInt();
    ship.pos = {static_cast<float>(json["pos"][0].GetDouble()),
                static_cast<float>(json["pos"][1].GetDouble())};
    ship.nextPointIdx = static_cast<size_t>(json["nextPointIdx"].GetInt());
    ship.people = json["people"].GetInt();
    ship.reached = true;

    return ship;
}
