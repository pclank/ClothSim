#pragma once

#include <Shader.hpp>
#include <vector>
#include <glm/glm.hpp>

struct ClothMesh {
	float width, depth, widthStep, depthStep;
	std::vector<glm::vec3> vertices, initVertices;
	std::vector<unsigned int> indices;
	unsigned int VAO, VBO, EBO;

	ClothMesh(float width, float depth, unsigned int wP, unsigned int dP, float initHeight = 2.0f)
		:
		width(width), depth(depth)
	{
		float w2 = width * 0.5f;
		float d2 = depth * 0.5f;

		// calculate the steps for each quad / tri
		float widthStep = width / wP;
		float depthStep = depth / dP;

		unsigned int gridRes = 3;

		// Calculate vertices
		unsigned int dI = 0;
		//for (float d = 0.0f; d < width; d += depthStep)
		for (float d = 0.0f; dI < gridRes; d += depthStep, dI++)
		{
			unsigned int wI = 0;
			for (float w = 0.0f; wI < gridRes; w += widthStep, wI++)
			{
				glm::vec3 tempVertex;

				tempVertex.y = initHeight;

				tempVertex.x = w;
				tempVertex.z = d;
				vertices.push_back(tempVertex);
			}
		}

		// Calculate indices
		unsigned int indexStart = 0;
		for (size_t i = 0; i < (gridRes - 1) * (gridRes - 1); i++)
		{
			if (i != 0 && i % (gridRes - 1) == 0)
				indexStart++;

			indices.push_back(indexStart);
			indices.push_back(indexStart + 1);
			indices.push_back(indexStart + 4);
			indices.push_back(indexStart + 3);

			indexStart++;
		}

		// Store initial positions
		initVertices = std::vector<glm::vec3>(vertices);

		/*for (size_t i = 0; i < initVertices.size(); i++)
			std::cout << initVertices[i].x << " | " << initVertices[i].y << " | " << initVertices[i].z << std::endl;*/

		// Set up buffers
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_DYNAMIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_DYNAMIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindVertexArray(0);

		std::cout << "Created cloth mesh with " << vertices.size() << " vertices and " << indices.size() << " indices" << std::endl;
	}

	void UpdateVertices(float time)
	{
		for (size_t i = 0; i < vertices.size(); i += 3)
		{
			vertices[i].y = initVertices[i].y + glm::cos(time);
		}

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_DYNAMIC_DRAW);
	}

	void Render(Shader& shader, glm::mat4 model)
	{
		shader.use();
		shader.setMat4("model", model);

		glDisable(GL_CULL_FACE);

		glBindVertexArray(VAO);

		//glDrawArrays(GL_TRIANGLES, 0, vertices.size());
		//glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
		glDrawElements(GL_TRIANGLE_FAN, indices.size(), GL_UNSIGNED_INT, 0);

		glBindVertexArray(0);

		glEnable(GL_CULL_FACE);
	}
};