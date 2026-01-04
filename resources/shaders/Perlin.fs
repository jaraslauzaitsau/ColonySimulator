// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#version 330

uniform float uScale;
uniform vec2 uResolution;
uniform vec2 uOffset;

uniform int uSeed;
uniform vec2 uMapSize;

uniform float uBiomeStart[8];
uniform vec4 uBiomeColor[8];
uniform int uBiomeCount;

float lerp(float start, float end, float amount) { return start + amount * (end - start); }

float fade(float t) { return t * t * t * (t * (t * 6 - 15) + 10); }

float rand(vec2 p)
{
    const float OFFSET = 100000.0;

    uint ix = uint(floor(p.x + OFFSET)) & 0xFFFFu;
    uint iy = uint(floor(p.y + OFFSET)) & 0xFFFFu;
    uint s = uint(floor(uSeed + 1000.0));

    uint n = ix * 73856093u + iy * 19349669u + s * 83492791u;

    n = ((n >> 16u) ^ n) * 0x45d9f3bu;
    n = ((n >> 16u) ^ n) * 0x45d9f3bu;
    n = (n >> 16u) ^ n;

    return float(n) * (1.0 / 4294967296.0);
}

vec2 gradient(vec2 p)
{
    float a = rand(p) * 6.2831853;
    return vec2(cos(a), sin(a));
}

float perlin(vec2 p)
{
    vec2 i0 = floor(p);
    vec2 f0 = fract(p);

    vec2 i1 = i0 + vec2(1.0);

    vec2 g00 = gradient(i0);
    vec2 g10 = gradient(vec2(i1.x, i0.y));
    vec2 g01 = gradient(vec2(i0.x, i1.y));
    vec2 g11 = gradient(i1);

    float n00 = dot(g00, f0);
    float n10 = dot(g10, vec2(f0.x - 1.0, f0.y));
    float n01 = dot(g01, vec2(f0.x, f0.y - 1.0));
    float n11 = dot(g11, vec2(f0.x - 1.0, f0.y - 1.0));

    float u = fade(f0.x);
    float v = fade(f0.y);

    float nx0 = lerp(n00, n10, u);
    float nx1 = lerp(n01, n11, u);

    return lerp(nx0, nx1, v);
}

float getPerlin(vec2 v)
{
    return (0.3 * perlin(v) + 2 * perlin(0.1 * v) + 3.5 * perlin(0.05 * v)) / 4.20;
}

vec4 applyBiomes(float v)
{
    vec4 color = uBiomeColor[0];

    for (int i = 1; i < uBiomeCount; i++)
    {
        if (v >= uBiomeStart[i])
        {
            float t = (v - uBiomeStart[i - 1]) / (uBiomeStart[i] - uBiomeStart[i - 1]);
            color = mix(uBiomeColor[i - 1], uBiomeColor[i], t);
        }
    }

    return color;
}

out vec4 fragColor;

void main()
{
    vec2 uv = (gl_FragCoord.xy - uResolution / 2) * uScale + uOffset;

    if (uv.x < -uMapSize.x / 2 || uv.x >= uMapSize.x / 2 || uv.y < -uMapSize.y / 2 ||
        uv.y >= uMapSize.y / 2)
    {
        fragColor = vec4(0, 0, 0, 1);
        return;
    }

    float n = getPerlin(uv);
    fragColor = applyBiomes(n);
}
