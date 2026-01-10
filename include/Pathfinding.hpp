// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <vector>

typedef struct Vector2 Vector2;

using Path = std::vector<Vector2>;
using ParentMap = std::vector<int>;

extern std::vector<ParentMap> pathMap;

void GeneratePathMap();
Path GetPath(Vector2 startPos, int targetIslandIdx);
