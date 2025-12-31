// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

typedef struct Vector2 Vector2;

extern bool vsync;
extern bool showFPS;
extern float panSensitivity;
extern float wheelSensitivity;
extern Vector2 mapSize;

void Save();
void Load();
