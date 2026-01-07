// SPDX-FileCopyrightText: 2025 Jaraslau Zaitsau
// SPDX-FileContributor: SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "Pathfinding.hpp"
#include <raylib.h>

#define SHIP_SPEED 25.0f

struct Ship
{
    size_t sourceIndex = 0;
    size_t targetIndex = 0;
    Vector2 pos{0, 0};
    float rotation = 0;
    Path path{};
    Vector2 nextPointDir{0, 0};
    size_t nextPointIdx = 0;
    int people = 0;
    bool reached = false;

    Ship(size_t sourceIndex, size_t targetIndex, int peopleCount = 1);
    void Move(float deltaTime);
};

extern std::vector<Ship> ships;
