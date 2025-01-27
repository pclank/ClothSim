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

		for (float d = -d2; d < d2; d += depthStep)
			for (float w = -w2; w < w2; w += widthStep)
			{
				glm::vec3 tempVertex;

				tempVertex.y = initHeight;

				unsigned int indexStart = vertices.size();

				tempVertex.x = w;
				tempVertex.z = d + depthStep;
				vertices.push_back(tempVertex);
				indices.push_back(indexStart++);

				tempVertex.x = w + widthStep;
				tempVertex.z = d + depthStep;
				vertices.push_back(tempVertex);
				indices.push_back(indexStart++);

				tempVertex.x = w + widthStep;
				tempVertex.z = d;
				vertices.push_back(tempVertex);
				indices.push_back(indexStart++);

				tempVertex.x = w;
				tempVertex.z = d;
				vertices.push_back(tempVertex);
				indices.push_back(indexStart);

				//// Define first triangle
				//tempVertex.x = w;
				//tempVertex.z = d + depthStep;
				//vertices.push_back(tempVertex);
				//indices.push_back(indexStart++);
				//
				//tempVertex.x = w + widthStep;
				//tempVertex.z = d + depthStep;
				//vertices.push_back(tempVertex);
				//indices.push_back(indexStart++);

				//tempVertex.x = w;
				//tempVertex.z = d;
				//vertices.push_back(tempVertex);
				//indices.push_back(indexStart++);

				//// Define second triangle
				//indices.push_back(indices[indices.size() - 2]);

				//tempVertex.x = w + widthStep;
				//tempVertex.z = d;
				//vertices.push_back(tempVertex);
				//indices.push_back(indexStart);

				//indices.push_back(indices[indices.size() - 3]);

				/*tempVertex.x = w + widthStep;
				tempVertex.z = d + depthStep;
				vertices.push_back(tempVertex);

				tempVertex.x = w + widthStep;
				tempVertex.z = d;
				vertices.push_back(tempVertex);

				tempVertex.x = w;
				tempVertex.z = d;
				vertices.push_back(tempVertex);*/
			}

		// Clean up duplicates
		std::vector<glm::vec3> vertCopy = std::vector<glm::vec3>(vertices);
		std::vector<std::pair<unsigned int, unsigned int>> duplicateIndices;	// [correct_index, duplicate_index]
		for (size_t i = 0; i < vertCopy.size() - 1; i++)
		{
			// Search for duplicates
			for (size_t j = i + 1; j < vertCopy.size(); j++)
				if (vertCopy[i] == vertCopy[j])
					duplicateIndices.push_back(std::pair<unsigned int, unsigned int>(i, j));
		}

		for (size_t i = 0; i < vertCopy.size() - 1; i++)
		{
			// Search for duplicates
			for (size_t j = i + 1; j < vertCopy.size(); j++)
				if (vertCopy[i] == vertCopy[j])
					vertCopy.erase(vertCopy.begin() + j);			// Erase duplicate from vertices
		}

		for (size_t i = 0; i < duplicateIndices.size(); i++)
		{
			// Fix indices
			for (size_t j = 0; j < indices.size(); j++)
			{
				if (indices[j] == duplicateIndices[i].second)
				{
					std::cout << indices[j] << " -> " << duplicateIndices[i].first << std::endl;
					indices[j] = duplicateIndices[i].first;

					//for (size_t k = 0;)
				}
			}
		}

		vertices = std::vector<glm::vec3>(vertCopy);

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