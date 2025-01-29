#pragma once

#include <cstdlib>
#include <glm/glm.hpp>

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
