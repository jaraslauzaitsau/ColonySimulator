// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "Island.hpp"

#include "Human.hpp"
#include "Perlin.hpp"

#define MIN_ANGLE -15
#define MAX_ANGLE 15

std::vector<Human> people;

void Human::MoveToTarget(double deltaTime)
{
    Vector2 delta = Vector2Rotate({speed, 0}, rotation) * deltaTime;
    while (GetPerlin(pos + delta) < LAND_START)
    {
        rotation = GetRandomFloat(0, 360);
        delta = Vector2Rotate({speed, 0}, rotation) * deltaTime;
    }
    pos += delta;

    angle += angleMultiplier * rotationSpeed * deltaTime;
    if (angle < MIN_ANGLE) angleMultiplier = 1;
    if (angle > MAX_ANGLE) angleMultiplier = -1;
}

JSON Human::ToJSON()
{
    JSON json;

    json["pos"].format = JSONFormat::Inline;
    json["pos"].push_back(pos.x);
    json["pos"].push_back(pos.y);

    json["angle"] = angle;
    json["rotation"] = rotation;
    json["islandIdx"] = islandIdx;
    json["speed"] = speed;
    json["rotationSpeed"] = rotationSpeed;

    return json;
}

Human Human::LoadJSON(JSON& json)
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
