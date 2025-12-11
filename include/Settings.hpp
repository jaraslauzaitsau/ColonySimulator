// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <raymath.h>

extern bool vsync;
extern bool showFPS;
extern double perlinScale;
extern double panSensitivity;
extern double wheelSensitivity;
extern Vector2 mapSize;

void Save();
void Load();
