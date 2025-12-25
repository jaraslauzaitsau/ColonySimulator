// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <raylib.h>

extern bool shouldClose;

enum class Menu
{
    Main,
    Game,
    Pause,
    Settings
};
extern Menu currentMenu;

extern Vector2 windowSize;

extern Shader biomeShader;
extern Shader islandShader;

extern Texture lockTexture;
extern Texture woodTexture;
extern Texture ironTexture;
extern Texture humanTexture;

Vector2 RaylibToGlsl(Vector2 v);
Vector2 GlslToRaylib(Vector2 v);

void UpdateWindowSize();
void ReloadIslandShaderValues();
void InitGPU();
void DrawFrame();
void FreeResources();
