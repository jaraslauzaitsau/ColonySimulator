// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <raylib.h>

extern Shader biomeShader;
extern Shader islandShader;

extern Texture lockTexture;
extern Texture woodTexture;
extern Texture ironTexture;
extern Texture humanTexture;

void DrawGameMenu();
void ProcessPlayerInput(double deltaTime);
