#pragma once

#include <glm/glm.hpp>
#include <math.h>
#include <random>

const float PI = 3.14159265358979323846264f;

inline float SmoothstepInterpolation(float t)
{
    return t* t* t* (t * (t * 6.0f - 15.0f) + 10.0f);
}

inline float Lerp(float a, float b, float x)
{
    return a + x * (b - a);
}

inline glm::vec2 GenerateGradient(float h)
{
    return glm::vec2(cos(h), sin(h));
}

inline float PerlinNoise(float x, float y, float gridSize)
{
    int x0 = floorf(x / gridSize);
    int y0 = floorf(y / gridSize);

    int x1 = x0 + 1;
    int y1 = y0 + 1;

    float dx = x / gridSize - x0;
    float dy = y / gridSize - y0;

    // Generate gradients
    glm::vec2 gradients[4];
    for (int i = 0; i < 4; i++)
        gradients[i] = GenerateGradient((static_cast<float>(rand()) / RAND_MAX) * 2 * PI);

    float dot00 = gradients[0].x * dx + gradients[0].y * dy;
    float dot10 = gradients[1].x * (dx - 1) + gradients[1].y * dy;
    float dot01 = gradients[2].x * dx + gradients[2].y * (dy - 1);
    float dot11 = gradients[3].x * (dx - 1) + gradients[3].y * (dy - 1);

    float u = SmoothstepInterpolation(dx);
    float v = SmoothstepInterpolation(dy);

    return Lerp(Lerp(dot00, dot10, u), Lerp(dot01, dot11, u), v);
}
