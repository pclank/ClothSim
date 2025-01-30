#pragma once

#include <cstdlib>
#include <glm/glm.hpp>

// **********************************************************************
// Constants and definitions
// **********************************************************************

const float epsilon = 0.001f;

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
