// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <raylib.h>

extern Vector2 windowSize;

Vector2 RaylibToGlsl(Vector2 v);
Vector2 GlslToRaylib(Vector2 v);

void InitGPU();
void DrawFrame();
void FreeResources();
