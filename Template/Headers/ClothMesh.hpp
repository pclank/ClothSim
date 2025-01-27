#pragma once

#include <Shader.hpp>
#include <vector>
#include <glm/glm.hpp>

#define GRAVITY 0.003f
#define VERLET_STEPS 3
#define CONSTRAINT_STEPS 4

const glm::vec3 gravity(0.0f, -GRAVITY, 0.0f);

const int xOffsets[4] = { 1, -1, 0, 0 };
const int yOffsets[4] = { 0, 0, 1, -1 };

struct ClothMesh {
	float width, depth, widthStep, depthStep;
	std::vector<glm::vec3> vertices, preVertices;
	std::vector<unsigned int> indices;
	std::vector<float> restLengths;					// 4 (except edges) initial distances to neigthbors
	unsigned int VAO, VBO, EBO;
	unsigned int gridRes;

	ClothMesh(float width, float depth, unsigned int wP, unsigned int dP, unsigned int gridRes, float initHeight = 2.0f)
		:
		width(width), depth(depth), gridRes(gridRes)
	{
		// calculate the steps for each quad
		widthStep = width / wP;
		depthStep = depth / dP;

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
			indices.push_back(indexStart + gridRes + 1);
			indices.push_back(indexStart + gridRes);

			indexStart++;
		}

		// Store initial positions
		preVertices = std::vector<glm::vec3>(vertices);

		/*for (size_t i = 0; i < preVertices.size(); i++)
			std::cout << preVertices[i].x << " | " << preVertices[i].y << " | " << preVertices[i].z << std::endl;*/

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

		// Calculate rest length
		for (size_t y = 1; y < gridRes - 1; y++)
			for (size_t x = 1; x < gridRes - 1; x++)
			{
				// 15% slack
				for (int c = 0; c < 4; c++)
				{
					glm::length(vertices[x + y * gridRes] - vertices[x + xOffsets[c] + (y + yOffsets[c]) * gridRes]) * 1.15f;
				}
			}

		std::cout << "Created cloth mesh with " << vertices.size() << " vertices and " << indices.size() << " indices" << std::endl;
	}

	~ClothMesh()
	{}

	inline void ApplyGravity(float dt)
	{
		for (size_t y = 1; y < gridRes; y++)
			for (size_t x = 0; x < gridRes; x++)
			{
				const glm::vec3 currentPos = vertices[x + y * gridRes];
				const glm::vec3 prevPos = preVertices[x + y * gridRes];

				vertices[x + y * gridRes] += (currentPos - prevPos) + gravity * dt;

				preVertices[x + y * gridRes] = currentPos;

				// if (Rand( 10 ) < 0.03f) grid( x, y ).pos += float2( Rand( 0.02f + magic ), Rand( 0.12f ) );
			}
	}

	inline void ApplyConstraints(float dt)
	{

	}

	void UpdateVertices(float time)
	{
		/*for (size_t i = 0; i < vertices.size(); i += 3)
		{
			vertices[i].y = preVertices[i].y + glm::cos(time);
		}*/

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