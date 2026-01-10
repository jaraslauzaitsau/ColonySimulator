// SPDX-FileCopyrightText: 2025 Jaraslau Zaitsau
// SPDX-FileContributor: SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "Json.hpp"
#include "Pathfinding.hpp"
#include <raylib.h>

#define SHIP_SPEED 25

struct Ship
{
    int sourceIndex = 0;
    int targetIndex = 0;
    Vector2 pos{0, 0};
    int flip = 1;
    Path path{};
    Vector2 nextPointDir{0, 0};
    size_t nextPointIdx = 0;
    int people = 0;
    bool reached = false;

    Ship() = default;
    Ship(int sourceIndex, int targetIndex, int peopleCount = 1);
    void Move(float deltaTime);

    Json ToJSON();
    static Ship LoadJSON(Json& json);
};

extern std::vector<Ship> ships;
