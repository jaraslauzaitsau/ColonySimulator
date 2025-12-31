// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <vector>
typedef struct Vector2 Vector2;

typedef std::vector<Vector2> Path;

Path FindPath(Vector2 start, Vector2 goal, bool onLand, float stepSize = 0.1f);
