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

	glm::vec3 GetNormal(glm::vec3& point)
	{
		// TODO: Handle same position case! Also in collision!
		return glm::normalize(point - pos);
	}

	/// <summary>
	/// Returns a std::pair with whether there was a collision, and the impulse required to resolve the collision
	/// </summary>
	/// <param name="vertexPos"></param>
	/// <param name="vertexMatrix"></param>
	/// <returns></returns>
	std::pair<bool, glm::vec3> CheckVertexCollision(glm::vec3& vertexPos, glm::mat4& vertexMatrix)
	{
		glm::vec3 transformedVertexPos = glm::vec3(vertexMatrix * (glm::vec4(vertexPos, 1.0f)));

		//std::pair<glm::vec3, float> intersectionData = IntersectSphere(vertexPos, pos, GetNormal(vertexPos), radius);
		std::pair<glm::vec3, float> intersectionData = IntersectSphere(transformedVertexPos, pos, GetNormal(transformedVertexPos), radius);

		//const glm::vec3 exitPoint = pos + GetNormal(vertexPos) * (intersectionData.second + epsilon);
		const glm::vec3 exitPoint = pos + GetNormal(transformedVertexPos) * (intersectionData.second + epsilon);
		const glm::vec3 impulse = exitPoint - pos;

		if (!isfinite(glm::length(impulse)))
		{
			throw std::runtime_error("non-finite impulse!");
		}

		return std::pair<bool, glm::vec3>( (intersectionData.second != 0.0f) && (intersectionData.first != glm::vec3(0.0f)),
			impulse);
	}
};
