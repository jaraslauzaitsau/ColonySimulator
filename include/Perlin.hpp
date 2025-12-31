// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

typedef struct Vector2 Vector2;

float GetPerlin(Vector2 v);
bool InsideMap(Vector2 pos);

extern int perlinSeed;
extern float perlinScale;
extern Vector2 perlinOffset;
