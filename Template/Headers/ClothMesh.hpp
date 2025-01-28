#pragma once

#include <Shader.hpp>
#include <vector>
#include <array>
#include <glm/glm.hpp>

#define GRAVITY 0.003f
#define VERLET_STEPS 3
#define CONSTRAINT_STEPS 4

const glm::vec3 gravity(0.0f, -GRAVITY, 0.0f);

const int xOffsets[4] = { 1, -1, 0, 0 };
const int yOffsets[4] = { 0, 0, 1, -1 };

struct ClothMesh {
	float width, depth, widthStep, depthStep;
	std::vector<glm::vec3> vertices, preVertices, fixedVertices;
	std::vector<unsigned int> indices, triIndices;
	std::vector<std::array<float, 4>> restLengths;				// 4 (except edges) initial distances to neigthbors
	std::array<float, 2> leftCornerRestLengths, rightCornerRestLengths;
	std::vector<std::array<float, 3>> leftRestLengths, rightRestLengths;
	unsigned int VAO, VBO, EBO;
	unsigned int gridRes;
	std::map<unsigned int, unsigned int> restMap;	// Maps vertex coordinates (x + y * gridRes) to restLength indices

	ClothMesh(float width, float depth, unsigned int wP, unsigned int dP, unsigned int gridRes, float initHeight = 2.0f)
		:
		width(width), depth(depth), gridRes(gridRes)
	{
		// calculate the steps for each quad
		widthStep = width / wP;
		depthStep = depth / dP;

		// Calculate vertices
		unsigned int dI = 0;
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

		// Calculate triangle indices
		for (size_t i = 0; i < indices.size(); i += 4)
		{
			triIndices.push_back(indices[i]);
			triIndices.push_back(indices[i + 1]);
			triIndices.push_back(indices[i + 2]);

			triIndices.push_back(indices[i]);
			triIndices.push_back(indices[i + 2]);
			triIndices.push_back(indices[i + 3]);
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
		//glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_DYNAMIC_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, triIndices.size() * sizeof(unsigned int), triIndices.data(), GL_DYNAMIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindVertexArray(0);

		// Add fixed vertex positions
		for (size_t x = 0; x < gridRes; x++)
			fixedVertices.push_back(vertices[x]);

		// Calculate rest length
		restLengths.resize(vertices.size() - 4 * gridRes + 4);
		unsigned int restIndex = 0;
		for (size_t y = 1; y < gridRes - 1; y++)
			for (size_t x = 1; x < gridRes - 1; x++)
			{
				// 15% slack
				for (int c = 0; c < 4; c++)
				{
					restLengths[restIndex][c] = glm::length(vertices[x + y * gridRes] -
						vertices[x + xOffsets[c] + (y + yOffsets[c]) * gridRes]) * 1.15f;
				}

				// Add to map
				restMap[x + y * gridRes] = restIndex;

				restIndex++;
			}

		// Calculate side vertices rest lengths
		for (unsigned int y = 1; y < gridRes - 1; y++)
		{
			std::array<float, 3> tmpLengths = {
				glm::length(vertices[y * gridRes] - vertices[1 + y * gridRes]) * 1.15f,					// Right neighbor
				glm::length(vertices[y * gridRes] - vertices[(y - 1) * gridRes]) * 1.15f,				// Top neighbor
				glm::length(vertices[y * gridRes] - vertices[(y + 1) * gridRes]) * 1.15f				// Bottom neighbor
			};
			leftRestLengths.push_back(tmpLengths);

			tmpLengths = {
				glm::length(vertices[gridRes - 1 + y * gridRes] - vertices[gridRes - 2 + y * gridRes]) * 1.15f,		// Left neighbor
				glm::length(vertices[gridRes - 1 + y * gridRes] - vertices[gridRes - 1 + (y - 1) * gridRes]) * 1.15f,				// Top neighbor
				glm::length(vertices[gridRes - 1 + y * gridRes] - vertices[gridRes - 1 + (y + 1) * gridRes]) * 1.15f				// Bottom neighbor
			};
			rightRestLengths.push_back(tmpLengths);
		}


		// Calculate bottom corner rest lengths
		leftCornerRestLengths = {
			glm::length(vertices[(gridRes - 1) * gridRes] - vertices[1 + (gridRes - 1) * gridRes]) * 1.15f,		// Right neighbor
			glm::length(vertices[(gridRes - 1) * gridRes] -	vertices[(gridRes - 2) * gridRes]) * 1.15f };		// Top neighbor

		rightCornerRestLengths = {
			glm::length(vertices[(gridRes - 1) + (gridRes - 1) * gridRes] - vertices[(gridRes - 2) + (gridRes - 1) * gridRes]) * 1.15f,		// Left neighbor
			glm::length(vertices[(gridRes - 1) + (gridRes - 1) * gridRes] - vertices[(gridRes - 1) + (gridRes - 2) * gridRes]) * 1.15f };	// Top neighbor };								// Top neighbor

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
				//vertices[x + y * gridRes] += (currentPos - prevPos) + gravity;

				preVertices[x + y * gridRes] = currentPos;

				// if (Rand( 10 ) < 0.03f) grid( x, y ).pos += float2( Rand( 0.02f + magic ), Rand( 0.12f ) );
			}
	}

	inline void ApplyConstraints(float dt)
	{
		for (int i = 0; i < CONSTRAINT_STEPS; i++)
		{
			for (int y = 1; y < gridRes - 1; y++)
				for (int x = 1; x < gridRes - 1; x++)
				{
					glm::vec3 pos = vertices[x + y * gridRes];

					// use springs to four neighbouring points
					for (int linknr = 0; linknr < 4; linknr++)
					{
						//Point& neighbour = grid( x + xoffset[linknr], y + yoffset[linknr] );

						const unsigned int neighborIndex = x + xOffsets[linknr] + (y + yOffsets[linknr]) * gridRes;
						
						glm::vec3 neighbor = vertices[neighborIndex];

						float distance = glm::length(neighbor - pos);
						if (!isfinite(distance))
						{
							// warning: this happens; sometimes vertex positions 'explode'.
							// TODO: CLAMP!!!
							vertices[x + y * gridRes] = preVertices[x + y * gridRes];
							continue;
						}
						if (distance > restLengths[restMap.at(x + y * gridRes)][linknr])
						{
							// pull points together
							float force = distance / (restLengths[restMap.at(x + y * gridRes)][linknr]) - 1;
							glm::vec3 direction = neighbor - pos;
							//glm::vec3 direction = glm::normalize(neighbor - pos);
							glm::vec3 impulse = force * direction * 0.5f;
							pos += impulse;
							neighbor -= impulse;
							/*pos -= force * direction * 0.5f;
							neighbor += force * direction * 0.5f;*/
						}

						vertices[x + y * gridRes] = pos;
						vertices[neighborIndex] = neighbor;
					}
				}

			// Constrain bottom corners
			const unsigned int rightIndex = 1 + (gridRes - 1) * gridRes;
			const unsigned int topLeftIndex = (gridRes - 2) * gridRes;
			const unsigned int leftIndex = (gridRes - 2) + (gridRes - 1) * gridRes;
			const unsigned int topRightIndex = (gridRes - 1) + (gridRes - 2) * gridRes;

			const std::array<unsigned int, 2> leftCornerIndices = { rightIndex, topLeftIndex };
			const std::array<unsigned int, 2> rightCornerIndices = { leftIndex, topRightIndex };

			// Left corner
			for (unsigned int index = 0; index < 2; index++)
			{
				glm::vec3 leftPos = vertices[(gridRes - 1) * gridRes];
				glm::vec3 neighbor = vertices[leftCornerIndices[index]];

				float distance = glm::length(neighbor - leftPos);
				if (!isfinite(distance))
				{
					// warning: this happens; sometimes vertex positions 'explode'.
					// TODO: CLAMP!!!
					vertices[(gridRes - 1) * gridRes] = preVertices[(gridRes - 1) * gridRes];
					continue;
				}
				if (distance > leftCornerRestLengths[index])
				{
					// pull points together
					float force = distance / (leftCornerRestLengths[index]) - 1;
					glm::vec3 direction = neighbor - leftPos;
					//glm::vec3 direction = glm::normalize(neighbor - leftPos);
					glm::vec3 impulse = force * direction * 0.5f;
					leftPos += impulse;
					neighbor -= impulse;
					/*leftPos -= force * direction * 0.5f;
					neighbor += force * direction * 0.5f;*/
				}

				vertices[(gridRes - 1) * gridRes] = leftPos;
				vertices[leftCornerIndices[index]] = neighbor;
			}

			// Right corner
			for (unsigned int index = 0; index < 2; index++)
			{
				glm::vec3 rightPos = vertices[(gridRes - 1) + (gridRes - 1) * gridRes];
				glm::vec3 neighbor = vertices[rightCornerIndices[index]];

				float distance = glm::length(neighbor - rightPos);
				if (!isfinite(distance))
				{
					// warning: this happens; sometimes vertex positions 'explode'.
					// TODO: CLAMP!!!
					vertices[(gridRes - 1) + (gridRes - 1) * gridRes] = preVertices[(gridRes - 1) + (gridRes - 1) * gridRes];
					continue;
				}
				if (distance > rightCornerRestLengths[index])
				{
					// pull points together
					float force = distance / (rightCornerRestLengths[index]) - 1;
					glm::vec3 direction = neighbor - rightPos;
					//glm::vec3 direction = glm::normalize(neighbor - rightPos);
					glm::vec3 impulse = force * direction * 0.5f;
					rightPos += impulse;
					neighbor -= impulse;
					/*rightPos -= force * direction * 0.5f;
					neighbor += force * direction * 0.5f;*/
				}

				vertices[(gridRes - 1) + (gridRes - 1) * gridRes] = rightPos;
				vertices[rightCornerIndices[index]] = neighbor;
			}

			const std::array<glm::ivec2, 3> leftSideOffsets = { glm::ivec2(1, 0), glm::ivec2(0, -1), glm::ivec2(0, 1) };
			const std::array<glm::ivec2, 3> rightSideOffsets = { glm::ivec2(-1, 0), glm::ivec2(0, -1), glm::ivec2(0, 1) };

			for (unsigned int y = 1; y < gridRes - 1; y++)
			{
				glm::vec3 pos = vertices[y * gridRes];
				for (unsigned int index = 0; index < 3; index++)
				{
					unsigned int neighborIndex = leftSideOffsets[index].x + (y + leftSideOffsets[index].y) * gridRes ;
					glm::vec3 neighbor = vertices[neighborIndex];

					float distance = glm::length(neighbor - pos);
					if (!isfinite(distance))
					{
						// warning: this happens; sometimes vertex positions 'explode'.
						// TODO: CLAMP!!!
						vertices[y * gridRes] = preVertices[y * gridRes];
						continue;
					}
					if (distance > leftRestLengths[y - 1][index])
					{
						// pull points together
						float force = distance / (leftRestLengths[y - 1][index]) - 1;
						glm::vec3 direction = neighbor - pos;
						//glm::vec3 direction = glm::normalize(neighbor - pos);
						glm::vec3 impulse = force * direction * 0.5f;
						pos += impulse;
						neighbor -= impulse;
						/*pos -= force * direction * 0.5f;
						neighbor += force * direction * 0.5f;*/
					}

					vertices[y * gridRes] = pos;
					vertices[neighborIndex] = neighbor;
				}

				pos = vertices[gridRes - 1 + y * gridRes];
				for (unsigned int index = 0; index < 3; index++)
				{
					unsigned int neighborIndex = gridRes - 1 + rightSideOffsets[index].x + (y + rightSideOffsets[index].y) * gridRes;
					glm::vec3 neighbor = vertices[neighborIndex];

					float distance = glm::length(neighbor - pos);
					if (!isfinite(distance))
					{
						// warning: this happens; sometimes vertex positions 'explode'.
						// TODO: CLAMP!!!
						vertices[gridRes - 1 + y * gridRes] = preVertices[gridRes - 1 + y * gridRes];
						continue;
					}
					if (distance > rightRestLengths[y - 1][index])
					{
						// pull points together
						float force = distance / (rightRestLengths[y - 1][index]) - 1;
						glm::vec3 direction = neighbor - pos;
						//glm::vec3 direction = glm::normalize(neighbor - pos);
						glm::vec3 impulse = force * direction * 0.5f;
						pos += impulse;
						neighbor -= impulse;
						/*pos -= force * direction * 0.5f;
						neighbor += force * direction * 0.5f;*/
					}

					vertices[gridRes - 1 + y * gridRes] = pos;
					vertices[neighborIndex] = neighbor;
				}
			}

			// Fixed vertices
			/*for (int x = 0; x < gridRes; x++)
				vertices[x] = fixedVertices[x];*/
		}

		//std::cout << vertices[(gridRes - 1) * gridRes].x << " | " << vertices[(gridRes - 1) * gridRes].y << " | " << vertices[(gridRes - 1) * gridRes].z << std::endl;
	}

	void Simulate(float dt)
	{
		for (int step = 0; step < VERLET_STEPS; step++)
		{
			//ApplyGravity(dt * 0.005f);
			ApplyGravity(dt);
			ApplyConstraints(dt);
		}
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
		glDrawElements(GL_TRIANGLES, triIndices.size(), GL_UNSIGNED_INT, 0);
		//glDrawElements(GL_TRIANGLE_FAN, indices.size(), GL_UNSIGNED_INT, 0);

		glBindVertexArray(0);

		glEnable(GL_CULL_FACE);
	}
};