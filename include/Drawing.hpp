// SPDX-FileCopyrightText: 2025 SemkiShow
// SPDX-FileContributor: Jaraslau Zaitsau
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

typedef struct Vector2 Vector2;
typedef struct Shader Shader;
typedef struct Texture Texture;
typedef struct Font Font;

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

extern Shader perlinShader;
extern Shader islandShader;

extern Texture lockTexture;
extern Texture woodTexture;
extern Texture ironTexture;
extern Texture humanTexture;

extern Font myFont;
const float myFontSpacing = 1;

Vector2 RaylibToGlsl(Vector2 v);
Vector2 GlslToRaylib(Vector2 v);

void UpdateWindowSize();
void ReloadIslandShaderValues();
void InitGPU();
void DrawFrame();
void FreeResources();
