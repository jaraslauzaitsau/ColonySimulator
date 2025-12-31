// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "Human.hpp"
#include "Island.hpp"
#include "Perlin.hpp"
#include <raymath.h>

#define MIN_ANGLE -15
#define MAX_ANGLE 15

std::vector<Human> people;

void Human::MoveToTarget(double deltaTime)
{
    Vector2 delta = Vector2Rotate({speed, 0}, rotation) * deltaTime;
    bool found = false;
    for (size_t i = 0; i < 5; i++)
    {
        if (GetPerlin(pos + delta) >= LAND_START && InsideMap(pos + delta))
        {
            found = true;
            break;
        }
        rotation = GetRandomFloat(0, 360);
        delta = Vector2Rotate({speed, 0}, rotation) * deltaTime;
    }
    if (found) pos += delta;

    angle += angleMultiplier * rotationSpeed * deltaTime;
    if (angle < MIN_ANGLE) angleMultiplier = 1;
    if (angle > MAX_ANGLE) angleMultiplier = -1;
}

Json Human::ToJSON()
{
    Json json;

    json["pos"].format = JsonFormat::Inline;
    json["pos"].push_back(pos.x);
    json["pos"].push_back(pos.y);

    json["angle"] = angle;
    json["rotation"] = rotation;
    json["islandIdx"] = islandIdx;
    json["speed"] = speed;
    json["rotationSpeed"] = rotationSpeed;

    return json;
}

Human Human::LoadJSON(Json& json)
{
    Human human;

    human.pos = {static_cast<float>(json["pos"][0].GetDouble()),
                 static_cast<float>(json["pos"][1].GetDouble())};
    human.angle = json["angle"].GetDouble();
    human.rotation = json["rotation"].GetDouble();
    human.islandIdx = json["islandIdx"].GetInt();
    human.speed = json["speed"].GetDouble();
    human.rotationSpeed = json["rotationSpeed"].GetDouble();

    return human;
}
