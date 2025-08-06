#pragma once

#include <cstdlib>
#include <glm/glm.hpp>

// **********************************************************************
// Constants and definitions
// **********************************************************************

const float epsilon = 0.001f;

const float PI = 3.14159265358979323846264f;

// **********************************************************************
// Interpolations
// **********************************************************************

inline float SmoothstepInterpolation(float t)
{
	return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

inline void SmoothstepInterpolationSIMD(__m256& out8, __m256 t8)
{
	static const __m256 six8 = _mm256_set1_ps(6.0f);
	static const __m256 minusFifteen8 = _mm256_set1_ps(-15.0f);
	static const __m256 ten8 = _mm256_set1_ps(10.0f);

	__m256 firstPart8 = _mm256_mul_ps(t8, _mm256_mul_ps(t8, t8));
	__m256 secondPart8 = _mm256_add_ps(_mm256_mul_ps(t8, _mm256_fmadd_ps(t8, six8, minusFifteen8)), ten8);

	out8 = _mm256_mul_ps(firstPart8, secondPart8);
}

inline float Lerp(float a, float b, float x)
{
	return a + x * (b - a);
}

inline __m256 LerpSIMD(__m256 a8, __m256 b8, __m256 x8)
{
	return _mm256_add_ps(a8, _mm256_mul_ps(x8, _mm256_sub_ps(b8, a8)));
}

// **********************************************************************
// Random generators
// **********************************************************************

inline float Random()
{
	return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
}

inline float Random(float high)
{
	return static_cast<float>(rand()) / static_cast<float>(RAND_MAX / high);
}

inline float Random(float low, float high)
{
	return low + static_cast<float>(rand()) / static_cast<float>(RAND_MAX / (high - low));
}

inline glm::vec3 Random3f()
{
	return glm::vec3(Random(), Random(), Random());
}

inline glm::vec3 Random3f(float high)
{
	return glm::vec3(Random(high), Random(high), Random(high));
}

inline glm::vec3 Random3f(float low, float high)
{
	return glm::vec3(Random(low, high), Random(low, high), Random(low, high));
}

inline glm::vec2 Random2f()
{
	return glm::vec2(Random(), Random());
}

inline glm::vec2 Random2f(float high)
{
	return glm::vec2(Random(high), Random(high));
}

inline glm::vec2 Random2f(float low, float high)
{
	return glm::vec2(Random(low, high), Random(low, high));
}

// **********************************************************************
// Intersections
// **********************************************************************

/// <summary>
/// Checks intersection between vertex and spheres. Returns a std::pair of intersection point and depth
/// </summary>
/// <param name="pos"></param>
/// <param name="sphere"></param>
/// <returns></returns>
//inline std::pair<glm::vec3, float> IntersectSphere(glm::vec3& pos, Sphere& sphere)
inline static std::pair<glm::vec3, float> IntersectSphere(glm::vec3& pos, glm::vec3 spPos, glm::vec3 spNormal, float spR)
{
	const float vertexSphereDistance = glm::length(pos - spPos);

	if (vertexSphereDistance - spR <= 0.0f)
	{
		const float depth = fabs(vertexSphereDistance - spR) != 0.0f ? fabs(vertexSphereDistance - spR) : epsilon;
		const glm::vec3 intersectionPoint = spPos + spNormal * spR;

		if (!isfinite(depth))
		{
			throw std::runtime_error("non-finite depth");
		}

		return std::pair<glm::vec3, float>(intersectionPoint, depth);
	}
	else
		return std::pair<glm::vec3, float>(glm::vec3(0.0f), 0.0f);
}
