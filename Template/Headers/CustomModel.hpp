#pragma once

#include <glm/glm.hpp>
#include <Shader.hpp>
#include <vector>

//const std::vector<unsigned int> debugIndices = { 0, 1, 2, 2, 3, 0 };
const std::vector<unsigned int> debugIndices = { 0, 1, 3, 1, 2, 3 };

struct CustomModel {
	std::vector<float> vertices;
	std::vector<float> initVertices;
	unsigned int VAO, VBO, EBO;

	CustomModel(std::vector<float> vertexArray)
		:
		vertices(vertexArray),
		initVertices(vertexArray)
	{
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, debugIndices.size() * sizeof(unsigned int), debugIndices.data(), GL_DYNAMIC_DRAW);

		// Update vertices
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindVertexArray(0);

		std::cout << "Created custom model with " << vertices.size() / 3 << " vertices" << std::endl;
	}

	void UpdateVertices(float time)
	{
		for (size_t i = 0; i < vertices.size(); i += 2)
		{
			vertices[i] = initVertices[i] + glm::cos(time);
			vertices[i + 1] = initVertices[i + 1] + glm::cos(time);
		}

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);
	}

	void Render(Shader& shader, glm::mat4 model)
	{
		shader.use();
		shader.setMat4("model", model);

		glDisable(GL_CULL_FACE);

		glBindVertexArray(VAO);

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		glEnable(GL_CULL_FACE);
	}
};