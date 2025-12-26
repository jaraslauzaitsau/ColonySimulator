// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "JSON.hpp"
#include "Utils.hpp"
#include <raymath.h>

#define MIN_SPEED 0.2f
#define MAX_SPEED 3
#define MIN_ROT_SPEED 20
#define MAX_ROT_SPEED 100

struct Human
{
    Vector2 pos = {0, 0};
    float angle = 0;
    float rotation = 0;
    int islandIdx = -1;
    float speed = 0, rotationSpeed = 0;
    int angleMultiplier = 1;

    Human() = default;
    Human(Vector2 pos, int islandIdx) : pos(pos), islandIdx(islandIdx)
    {
        speed = GetRandomFloat(MIN_SPEED, MAX_SPEED);
        rotationSpeed = GetRandomFloat(MIN_ROT_SPEED, MAX_ROT_SPEED);
    }

    void MoveToTarget(double deltaTime);

    JSON ToJSON();
    static Human LoadJSON(JSON& json);
};

extern std::vector<Human> people;
