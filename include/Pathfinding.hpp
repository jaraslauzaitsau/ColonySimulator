// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <raymath.h>
#include <vector>

typedef std::vector<Vector2> Path;

Path FindPath(Vector2 start, Vector2 goal, bool onLand, float stepSize = 0.1f);
