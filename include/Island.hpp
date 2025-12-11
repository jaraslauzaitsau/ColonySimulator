// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <raymath.h>
#include <vector>

struct Island
{
    Vector2 p1 = {0, 0}, p2 = {0, 0};
    int treeCount = 0, ironCount = 0;
};

extern std::vector<Island> islands;
