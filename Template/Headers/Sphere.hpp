#pragma once

#include <utility>
#include <glm/glm.hpp>
#include <ExtraMath.hpp>

struct Sphere {
	glm::vec3 pos;
	float radius;
	glm::mat4 transformationMatrix;

	Sphere(glm::vec3 pos, float radius)
		:
		pos(pos), radius(radius)
	{
		transformationMatrix = glm::mat4(1.0f);
	}

	/// <summary>
	/// Returns a std::pair with whether there was a collision, and the impulse required to resolve the collision
	/// </summary>
	/// <param name="vertexPos"></param>
	/// <param name="vertexMatrix"></param>
	/// <returns></returns>
	std::pair<bool, glm::vec3> CheckVertexCollision(glm::vec3& vertexPos, glm::mat4& vertexMatrix)
	{
		std::pair<glm::vec3, float> intersectionData = IntersectSphere(vertexPos, pos, radius);

		// TODO: TEST ONLY!
		return std::pair<bool, glm::vec3>( (intersectionData.second != 0.0f) && (intersectionData.first != glm::vec3(0.0f)),
			glm::vec3(1.0f));
	}
};
