#pragma once

#include <glm/glm.hpp>
#include <math.h>
#include <random>
#include <ExtraMath.hpp>

inline glm::vec2 GenerateGradient(float h)
{
    return glm::vec2(cos(h), sin(h));
}

inline void GenerateGradientSIMD(__m256& out_x8, __m256& out_y8)
{
    // Proper randomness
    __m256 height_8 = _mm256_setr_ps((static_cast<float>(rand()) / RAND_MAX) * 2 * PI,
        (static_cast<float>(rand()) / RAND_MAX) * 2 * PI,
        (static_cast<float>(rand()) / RAND_MAX) * 2 * PI,
        (static_cast<float>(rand()) / RAND_MAX) * 2 * PI,
        (static_cast<float>(rand()) / RAND_MAX) * 2 * PI,
        (static_cast<float>(rand()) / RAND_MAX) * 2 * PI,
        (static_cast<float>(rand()) / RAND_MAX) * 2 * PI,
        (static_cast<float>(rand()) / RAND_MAX) * 2 * PI);

    out_x8 = _mm256_cos_ps(height_8);
    out_y8 = _mm256_sin_ps(height_8);
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

/// <summary>
/// Perlin noise sampling for SIMD (AVX2)
/// </summary>
/// <param name="x8"></param>
/// <param name="y8"></param>
/// <param name="gridSize"></param>
/// <returns></returns>
inline __m256 PerlinNoiseSIMD(__m256 x8, __m256 y8, float gridSize = 8)
{
    const __m256 gridSize8 = _mm256_set1_ps(gridSize);
    static const __m256 one8 = _mm256_set1_ps(1.0f);

    __m256 x0_8 = _mm256_floor_ps(_mm256_div_ps(x8, gridSize8));
    __m256 y0_8 = _mm256_floor_ps(_mm256_div_ps(y8, gridSize8));

    __m256 x1_8 = _mm256_add_ps(x0_8, one8);
    __m256 y1_8 = _mm256_add_ps(y0_8, one8);

    __m256 dx8 = _mm256_sub_ps(_mm256_div_ps(x8, gridSize8), x0_8);
    __m256 dy8 = _mm256_sub_ps(_mm256_div_ps(y8, gridSize8), y0_8);

    // Generate 4 gradients per input (4 * 8)
    __m256 gradient_x8[4], gradient_y8[4];
    for (int i = 0; i < 4; i++)
        GenerateGradientSIMD(gradient_x8[i], gradient_y8[i]);

    __m256 dot00_8 = _mm256_add_ps(_mm256_mul_ps(gradient_x8[0], dx8), _mm256_mul_ps(gradient_y8[0], dy8));
    __m256 dot10_8 = _mm256_add_ps(_mm256_mul_ps(gradient_x8[1], _mm256_sub_ps(dx8, one8)), _mm256_mul_ps(gradient_y8[1], dy8));
    __m256 dot01_8 = _mm256_add_ps(_mm256_mul_ps(gradient_x8[2], dx8), _mm256_mul_ps(gradient_y8[2], _mm256_sub_ps(dy8, one8)));
    __m256 dot11_8 = _mm256_add_ps(_mm256_mul_ps(gradient_x8[3], _mm256_sub_ps(dx8, one8)), _mm256_mul_ps(gradient_y8[3], _mm256_sub_ps(dy8, one8)));

    __m256 u8, v8;
    SmoothstepInterpolationSIMD(u8, dx8);
    SmoothstepInterpolationSIMD(v8, dy8);

    return LerpSIMD(LerpSIMD(dot00_8, dot10_8, u8), LerpSIMD(dot01_8, dot11_8, u8), v8);
}
