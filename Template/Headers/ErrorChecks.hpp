#pragma once
#include <immintrin.h>
#include <cmath>
#include <iostream>
#include <glm/glm.hpp>

inline bool IsInfSIMD(__m256& v8)
{
	for (int i = 0; i < 8; i++)
		if (isinf(v8.m256_f32[i]))
		{
			//std::cout << "value " << i << " was inf!";
			throw std::runtime_error("value was inf!");
		}

	return false;
}

inline bool IsNaNSIMD(__m256& v8)
{
	for (int i = 0; i < 8; i++)
		if (_isnan(v8.m256_f32[i]))
		{
			//std::cout << "value " << i << " was inf!";
			throw std::runtime_error("value was inf!");
		}

	return false;
}

inline bool IsInfVec3(glm::vec3& v)
{
	if (isinf(v.x) || isinf(v.y) || isinf(v.z))
		return true;

	return false;
}

inline bool IsNaNVec3(glm::vec3& v)
{
	if (isnan(v.x) || isnan(v.y) || isnan(v.z))
		return true;

	return false;
}
