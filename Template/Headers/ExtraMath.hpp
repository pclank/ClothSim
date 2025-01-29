#pragma once

#include <cstdlib>
#include <glm/glm.hpp>

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
inline static std::pair<glm::vec3, float> IntersectSphere(glm::vec3& pos, glm::vec3 spPos, float spR)
{
	glm::vec3 oc = -spPos;
	float b = dot(oc, pos);
	float c = dot(oc, oc) - spR * spR;

	float t, d = b * b - c;
	if (d <= 0)
	{
		return std::pair<glm::vec3, float>(glm::vec3(0.0f), 0.0f);
	}

	d = sqrt(d), t = -b - d;
	bool hit = t < glm::length(pos) && t > 0;
	if (hit)
	{
		return std::pair<glm::vec3, float>(pos - glm::normalize(pos) * fabs(t - glm::length(pos)),
			fabs(t - glm::length(pos)));
	}

	// we're outside; safe to skip option 2
	if (c > 0)
	{
		return std::pair<glm::vec3, float>(glm::vec3(0.0f), 0.0f);
	};

	t = d - b, hit = t < glm::length(pos) && t > 0;
	if (hit)
	{
		return std::pair<glm::vec3, float>(pos - glm::normalize(pos) * fabs(t - glm::length(pos)),
			fabs(t - glm::length(pos)));
	}

	return std::pair<glm::vec3, float>(glm::vec3(0.0f), 0.0f);
}
