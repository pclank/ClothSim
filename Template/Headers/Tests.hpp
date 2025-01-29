#pragma once

#include <array>
#include <cstdlib>
#include <string>
#include <iostream>
#include <glm/glm.hpp>
#include <Sphere.hpp>

void SphereIntersectionTest(glm::vec3& pos, Sphere& sphere)
{
	const std::string log = std::to_string(pos.x) + " | " + std::to_string(pos.y) + " | " + std::to_string(pos.z);
	std::cout << log << std::endl;

	std::pair<bool, glm::vec3> collisionData = sphere.CheckVertexCollision(pos, glm::mat4(1.0f));

	if (collisionData.first)
	{
		const std::string res = "Collision detected with impulse: "
			+ std::to_string(collisionData.second.x) + " | " + std::to_string(collisionData.second.y) + " | " + std::to_string(collisionData.second.z);
		std::cout << res << std::endl;
	}
	else
		std::cout << "No collision detected" << std::endl;
}

void SphereIntersectionTesting()
{
	std::array<glm::vec3, 3> vertexInputs = { glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f) };
	Sphere sphereInput(glm::vec3(0.0f), 2.0f);
	
	for (size_t i = 0; i < 3; i++)
		SphereIntersectionTest(vertexInputs[i], sphereInput);
}