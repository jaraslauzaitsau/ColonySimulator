#version 330

uniform float uScale;
uniform vec2 uResolution;
uniform float uSeed;
uniform vec2 uOffset;

uniform float uBiomeStart[8];
uniform vec4 uBiomeColor[8];
uniform int uBiomeCount;

float fade(float t) { return t * t * t * (t * (t * 6 - 15) + 10); }

float rand(vec2 p) { return fract(sin(dot(p, vec2(127.1, 311.7)) + uSeed) * 43758.5453123); }

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
    vec2 f1 = f0 - vec2(1.0);

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

    float nx0 = mix(n00, n10, u);
    float nx1 = mix(n01, n11, u);

    return mix(nx0, nx1, v);
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
    vec2 uv = gl_FragCoord.xy * uScale;

    float n = perlin(uv + uOffset);
    fragColor = applyBiomes(n);
}
