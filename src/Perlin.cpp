// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "Perlin.hpp"
#include "Settings.hpp"
#include <cmath>
#include <raymath.h>

int perlinSeed = 0;
float perlinScale = 0.12f;
Vector2 perlinOffset = {0, 0};

float Fract(float x) { return x - floorf(x); }

float Fade(float t) { return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f); }

float RandPerlin(Vector2 p)
{
    const float OFFSET = 100000.0f;

    unsigned int ix = (unsigned int)floorf(p.x + OFFSET) & 0xFFFFu;
    unsigned int iy = (unsigned int)floorf(p.y + OFFSET) & 0xFFFFu;
    unsigned int s = (unsigned int)floorf(perlinSeed + 1000.0f);

    unsigned int n = ix * 73856093u + iy * 19349669u + s * 83492791u;

    n = ((n >> 16u) ^ n) * 0x45d9f3bu;
    n = ((n >> 16u) ^ n) * 0x45d9f3bu;
    n = (n >> 16u) ^ n;

    return (float)n * (1.0f / 4294967296.0f);
}

Vector2 Gradient(Vector2 p)
{
    float a = RandPerlin(p) * PI * 2;
    return {cosf(a), sinf(a)};
}

float Perlin(Vector2 p)
{
    Vector2 i0 = {floor(p.x), floor(p.y)};
    Vector2 f0 = {Fract(p.x), Fract(p.y)};

    Vector2 i1 = Vector2Add(i0, {1.0f, 1.0f});

    Vector2 g00 = Gradient(i0);
    Vector2 g10 = Gradient({i1.x, i0.y});
    Vector2 g01 = Gradient({i0.x, i1.y});
    Vector2 g11 = Gradient(i1);

    float n00 = Vector2DotProduct(g00, f0);
    float n10 = Vector2DotProduct(g10, {f0.x - 1.0f, f0.y});
    float n01 = Vector2DotProduct(g01, {f0.x, f0.y - 1.0f});
    float n11 = Vector2DotProduct(g11, {f0.x - 1.0f, f0.y - 1.0f});

    float u = Fade(f0.x);
    float v = Fade(f0.y);

    float nx0 = Lerp(n00, n10, u);
    float nx1 = Lerp(n01, n11, u);

    return Lerp(nx0, nx1, v);
}

float GetPerlin(Vector2 v)
{
    return (0.3f * Perlin(v) + 2.0f * Perlin(Vector2Scale(v, 0.1f)) +
            3.5f * Perlin(Vector2Scale(v, 0.05f))) /
           4.20f;
}

bool InsideMap(Vector2 pos)
{
    return pos.x > -mapSize.x / 2 && pos.x < mapSize.x / 2 && pos.y > -mapSize.y / 2 &&
           pos.y < mapSize.y / 2;
}
