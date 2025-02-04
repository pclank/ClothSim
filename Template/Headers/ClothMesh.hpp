#pragma once

#include <Shader.hpp>
#include <Sphere.hpp>
#include <ExtraMath.hpp>
#include <vector>
#include <array>
#include <direct.h>
#include <glm/glm.hpp>

//#define SPHERE_COLLISION

#define GRAVITY 0.003f
#define VERLET_STEPS 3
//#define CONSTRAINT_STEPS 4
#define CONSTRAINT_STEPS 10

struct SimpleVertex {
	glm::vec3 pos;
	glm::vec2 texCoords;

	SimpleVertex(glm::vec3 pos, glm::vec2 texCoords)
		:
		pos(pos), texCoords(texCoords)
	{}
};

const glm::vec3 gravity(0.0f, -GRAVITY, 0.0f);

const int xOffsets[4] = { 1, -1, 0, 0 };
const int yOffsets[4] = { 0, 0, 1, -1 };

struct ClothMesh {
	float width, depth, widthStep, depthStep, dU, dV;
	//std::vector<glm::vec3> vertices, preVertices, fixedVertices;
	std::vector<SimpleVertex> vertices, preVertices, fixedVertices;
	std::vector<glm::vec2> texCoords;
	std::vector<unsigned int> indices, triIndices;
	std::vector<std::array<float, 4>> restLengths;				// 4 (except edges) initial distances to neigthbors
	std::array<float, 2> leftCornerRestLengths, rightCornerRestLengths;
	std::vector<std::array<float, 3>> leftRestLengths, rightRestLengths;
	unsigned int VAO, VBO, EBO;
	unsigned int gridRes;
	unsigned int textureId;
	std::map<unsigned int, unsigned int> restMap;	// Maps vertex coordinates (x + y * gridRes) to restLength indices

	ClothMesh(float width, float depth, unsigned int wP, unsigned int dP, unsigned int gridRes,
		std::string textureFile = "clothTexture.jpg", float initHeight = 2.0f)
		:
		width(width), depth(depth), gridRes(gridRes)
	{
		// Load texture
		char buffer[1024];
		getcwd(buffer, 1024);
		std::string texturePath(buffer);
		//texturePath += "\\..\\textures\\clothTexture.jpg";
		texturePath += "\\..\\textures\\" + textureFile;

		textureId = TextureFromFile(texturePath.c_str(), false);

		// Calculate the steps for each quad
		widthStep = width / wP;
		depthStep = depth / dP;

		// Calculate the steps for texCoords
		/*dU = 0.9f / wP;
		dV = 0.9f / wP;*/
		dU = 1.0f / (gridRes - 1);
		dV = 1.0f / (gridRes - 1);

		// Calculate vertices
		unsigned int dI = 0;
		for (float d = 0.0f; dI < gridRes; d += depthStep, dI++)
		{
			float v = dI * dV;
			unsigned int wI = 0;
			for (float w = 0.0f; wI < gridRes; w += widthStep, wI++)
			{
				glm::vec3 tempVertex;

				tempVertex.y = initHeight;

				tempVertex.x = w;
				tempVertex.z = d;

				//vertices.push_back(tempVertex);

				float u = wI * dU;

				texCoords.push_back(glm::vec2(u, v));

				SimpleVertex sv(tempVertex, glm::vec2(u, v));
				vertices.push_back(sv);
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
		//preVertices = std::vector<glm::vec3>(vertices);
		preVertices = std::vector<SimpleVertex>(vertices);

		/*for (size_t i = 0; i < preVertices.size(); i++)
			std::cout << preVertices[i].x << " | " << preVertices[i].y << " | " << preVertices[i].z << std::endl;*/

		// Set up buffers
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		//glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_DYNAMIC_DRAW);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(SimpleVertex), vertices.data(), GL_DYNAMIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		//glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_DYNAMIC_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, triIndices.size() * sizeof(unsigned int), triIndices.data(), GL_DYNAMIC_DRAW);

		// Vertex positions
		//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SimpleVertex), (void*)0);
		glEnableVertexAttribArray(0);

		// Vertex texCoords
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(SimpleVertex), (void*)offsetof(SimpleVertex, texCoords));
		glEnableVertexAttribArray(1);

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
					restLengths[restIndex][c] = glm::length(vertices[x + y * gridRes].pos -
						vertices[x + xOffsets[c] + (y + yOffsets[c]) * gridRes].pos) * 1.15f;
				}

				// Add to map
				restMap[x + y * gridRes] = restIndex;

				restIndex++;
			}

		// Calculate side vertices rest lengths
		for (unsigned int y = 1; y < gridRes - 1; y++)
		{
			std::array<float, 3> tmpLengths = {
				glm::length(vertices[y * gridRes].pos - vertices[1 + y * gridRes].pos) * 1.15f,					// Right neighbor
				glm::length(vertices[y * gridRes].pos - vertices[(y - 1) * gridRes].pos) * 1.15f,				// Top neighbor
				glm::length(vertices[y * gridRes].pos - vertices[(y + 1) * gridRes].pos) * 1.15f				// Bottom neighbor
			};
			leftRestLengths.push_back(tmpLengths);

			tmpLengths = {
				glm::length(vertices[gridRes - 1 + y * gridRes].pos - vertices[gridRes - 2 + y * gridRes].pos) * 1.15f,			// Left neighbor
				glm::length(vertices[gridRes - 1 + y * gridRes].pos - vertices[gridRes - 1 + (y - 1) * gridRes].pos) * 1.15f,	// Top neighbor
				glm::length(vertices[gridRes - 1 + y * gridRes].pos - vertices[gridRes - 1 + (y + 1) * gridRes].pos) * 1.15f	// Bottom neighbor
			};
			rightRestLengths.push_back(tmpLengths);
		}


		// Calculate bottom corner rest lengths
		leftCornerRestLengths = {
			glm::length(vertices[(gridRes - 1) * gridRes].pos - vertices[1 + (gridRes - 1) * gridRes].pos) * 1.15f,		// Right neighbor
			glm::length(vertices[(gridRes - 1) * gridRes].pos -	vertices[(gridRes - 2) * gridRes].pos) * 1.15f };		// Top neighbor

		rightCornerRestLengths = {
			glm::length(vertices[(gridRes - 1) + (gridRes - 1) * gridRes].pos - vertices[(gridRes - 2) + (gridRes - 1) * gridRes].pos) * 1.15f,		// Left neighbor
			glm::length(vertices[(gridRes - 1) + (gridRes - 1) * gridRes].pos - vertices[(gridRes - 1) + (gridRes - 2) * gridRes].pos) * 1.15f };	// Top neighbor };								// Top neighbor

		std::cout << "Created cloth mesh with " << vertices.size() << " vertices and " << triIndices.size() << " indices" << std::endl;
	}

	~ClothMesh()
	{}

	inline void ApplyGravity(float dt)
	{
		for (size_t y = 1; y < gridRes; y++)
			for (size_t x = 0; x < gridRes; x++)
			{
				const glm::vec3 currentPos = vertices[x + y * gridRes].pos;
				const glm::vec3 prevPos = preVertices[x + y * gridRes].pos;

				vertices[x + y * gridRes].pos += (currentPos - prevPos) + gravity * dt;
				//vertices[x + y * gridRes].pos += (currentPos - prevPos) + gravity;

				if (!isfinite(glm::length(vertices[x + y * gridRes].pos)))
				{
					throw std::runtime_error("gravity issue");
				}

				preVertices[x + y * gridRes].pos = currentPos;

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
					glm::vec3 pos = vertices[x + y * gridRes].pos;

					// Use springs constraint vertices
					for (int linknr = 0; linknr < 4; linknr++)
					{
						const unsigned int neighborIndex = x + xOffsets[linknr] + (y + yOffsets[linknr]) * gridRes;
						
						glm::vec3 neighbor = vertices[neighborIndex].pos;

						float distance = glm::length(neighbor - pos);
						if (!isfinite(distance))
						{
							// TODO: CLAMP!!!
							vertices[x + y * gridRes].pos = preVertices[x + y * gridRes].pos;
							continue;
						}
						if (distance > restLengths[restMap.at(x + y * gridRes)][linknr])
						{
							// Pull vertices closer
							float force = distance / (restLengths[restMap.at(x + y * gridRes)][linknr]) - 1;
							glm::vec3 direction = neighbor - pos;
							//glm::vec3 direction = glm::normalize(neighbor - pos);
							glm::vec3 impulse = force * direction * 0.5f;
							pos += impulse;
							neighbor -= impulse;
							/*pos -= force * direction * 0.5f;
							neighbor += force * direction * 0.5f;*/
						}

						vertices[x + y * gridRes].pos = pos;
						vertices[neighborIndex].pos = neighbor;
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
				glm::vec3 leftPos = vertices[(gridRes - 1) * gridRes].pos;
				glm::vec3 neighbor = vertices[leftCornerIndices[index]].pos;

				float distance = glm::length(neighbor - leftPos);
				if (!isfinite(distance))
				{
					// TODO: CLAMP!!!
					vertices[(gridRes - 1) * gridRes].pos = preVertices[(gridRes - 1) * gridRes].pos;
					continue;
				}
				if (distance > leftCornerRestLengths[index])
				{
					// Pull vertices closer
					float force = distance / (leftCornerRestLengths[index]) - 1;
					glm::vec3 direction = neighbor - leftPos;
					//glm::vec3 direction = glm::normalize(neighbor - leftPos);
					glm::vec3 impulse = force * direction * 0.5f;
					leftPos += impulse;
					neighbor -= impulse;
					/*leftPos -= force * direction * 0.5f;
					neighbor += force * direction * 0.5f;*/
				}

				vertices[(gridRes - 1) * gridRes].pos = leftPos;
				vertices[leftCornerIndices[index]].pos = neighbor;
			}

			// Right corner
			for (unsigned int index = 0; index < 2; index++)
			{
				glm::vec3 rightPos = vertices[(gridRes - 1) + (gridRes - 1) * gridRes].pos;
				glm::vec3 neighbor = vertices[rightCornerIndices[index]].pos;

				float distance = glm::length(neighbor - rightPos);
				if (!isfinite(distance))
				{
					// TODO: CLAMP!!!
					vertices[(gridRes - 1) + (gridRes - 1) * gridRes].pos = preVertices[(gridRes - 1) + (gridRes - 1) * gridRes].pos;
					continue;
				}
				if (distance > rightCornerRestLengths[index])
				{
					// Pull vertices closer
					float force = distance / (rightCornerRestLengths[index]) - 1;
					glm::vec3 direction = neighbor - rightPos;
					//glm::vec3 direction = glm::normalize(neighbor - rightPos);
					glm::vec3 impulse = force * direction * 0.5f;
					rightPos += impulse;
					neighbor -= impulse;
					/*rightPos -= force * direction * 0.5f;
					neighbor += force * direction * 0.5f;*/
				}

				vertices[(gridRes - 1) + (gridRes - 1) * gridRes].pos = rightPos;
				vertices[rightCornerIndices[index]].pos = neighbor;
			}

			const std::array<glm::ivec2, 3> leftSideOffsets = { glm::ivec2(1, 0), glm::ivec2(0, -1), glm::ivec2(0, 1) };
			const std::array<glm::ivec2, 3> rightSideOffsets = { glm::ivec2(-1, 0), glm::ivec2(0, -1), glm::ivec2(0, 1) };

			for (unsigned int y = 1; y < gridRes - 1; y++)
			{
				glm::vec3 pos = vertices[y * gridRes].pos;
				for (unsigned int index = 0; index < 3; index++)
				{
					unsigned int neighborIndex = leftSideOffsets[index].x + (y + leftSideOffsets[index].y) * gridRes ;
					glm::vec3 neighbor = vertices[neighborIndex].pos;

					float distance = glm::length(neighbor - pos);
					if (!isfinite(distance))
					{
						// TODO: CLAMP!!!
						vertices[y * gridRes].pos = preVertices[y * gridRes].pos;
						continue;
					}
					if (distance > leftRestLengths[y - 1][index])
					{
						// Pull vertices closer
						float force = distance / (leftRestLengths[y - 1][index]) - 1;
						glm::vec3 direction = neighbor - pos;
						//glm::vec3 direction = glm::normalize(neighbor - pos);
						glm::vec3 impulse = force * direction * 0.5f;
						pos += impulse;
						neighbor -= impulse;
						/*pos -= force * direction * 0.5f;
						neighbor += force * direction * 0.5f;*/
					}

					vertices[y * gridRes].pos = pos;
					vertices[neighborIndex].pos = neighbor;
				}

				pos = vertices[gridRes - 1 + y * gridRes].pos;
				for (unsigned int index = 0; index < 3; index++)
				{
					unsigned int neighborIndex = gridRes - 1 + rightSideOffsets[index].x + (y + rightSideOffsets[index].y) * gridRes;
					glm::vec3 neighbor = vertices[neighborIndex].pos;

					float distance = glm::length(neighbor - pos);
					if (!isfinite(distance))
					{
						// TODO: CLAMP!!!
						vertices[gridRes - 1 + y * gridRes].pos = preVertices[gridRes - 1 + y * gridRes].pos;
						continue;
					}
					if (distance > rightRestLengths[y - 1][index])
					{
						// Pull vertices closer
						float force = distance / (rightRestLengths[y - 1][index]) - 1;
						glm::vec3 direction = neighbor - pos;
						//glm::vec3 direction = glm::normalize(neighbor - pos);
						glm::vec3 impulse = force * direction * 0.5f;
						pos += impulse;
						neighbor -= impulse;
						/*pos -= force * direction * 0.5f;
						neighbor += force * direction * 0.5f;*/
					}

					vertices[gridRes - 1 + y * gridRes].pos = pos;
					vertices[neighborIndex].pos = neighbor;
				}
			}

			// Fixed vertices
			for (int x = 0; x < gridRes; x++)
				vertices[x].pos = fixedVertices[x].pos;
		}

		//std::cout << vertices[(gridRes - 1) * gridRes].x << " | " << vertices[(gridRes - 1) * gridRes].y << " | " << vertices[(gridRes - 1) * gridRes].z << std::endl;
	}

	void AddDrag(float drag, float dt)
	{
		for (size_t y = 1; y < gridRes; y++)
			for (size_t x = 0; x < gridRes; x++)
			{
				const glm::vec3 currentPos = vertices[x + y * gridRes].pos;
				const glm::vec3 prevPos = preVertices[x + y * gridRes].pos;

				//const glm::vec3 dragDirection = -(currentPos - prevPos);
				const glm::vec3 dragDirection = prevPos - currentPos;
				//const glm::vec3 dragDirection = (prevPos - currentPos) * (prevPos - currentPos);

				vertices[x + y * gridRes].pos += (currentPos - prevPos) + dragDirection * drag * dt;
				//vertices[x + y * gridRes].pos += (currentPos - prevPos) + dragDirection * drag;

				if (!isfinite(glm::length(vertices[x + y * gridRes].pos)))
				{
					throw std::runtime_error("drag issue");
				}

				preVertices[x + y * gridRes].pos = currentPos;
			}
	}

	void AddWind(float wind, float dt)
	{
		for (size_t y = 1; y < gridRes; y++)
			for (size_t x = 0; x < gridRes; x++)
			{
				const glm::vec3 currentPos = vertices[x + y * gridRes].pos;
				const glm::vec3 prevPos = preVertices[x + y * gridRes].pos;

				const glm::vec3 windDirection = glm::normalize(Random3f(-1.0f, 1.0f));

				vertices[x + y * gridRes].pos += (currentPos - prevPos) + windDirection * wind * dt;

				if (!isfinite(glm::length(vertices[x + y * gridRes].pos)))
				{
					throw std::runtime_error("wind issue");
				}

				//vertices[x + y * gridRes].pos += (currentPos - prevPos) + windDirection * wind;

				preVertices[x + y * gridRes].pos = currentPos;
			}
	}

	void Collide(glm::mat4 modelMatrix, float dt)
	{
		// TODO: Remove hardcoded sphere!
		Sphere sphere(glm::vec3(2.0f, 1.0f, 0.0f), 1.0f);

		for (size_t y = 1; y < gridRes; y++)
			for (size_t x = 0; x < gridRes; x++)
			{
				const glm::vec3 currentPos = vertices[x + y * gridRes].pos;

				//std::pair<bool, glm::vec3> collisionData = sphere.CheckVertexCollision(vertices[x + y * gridRes].pos, glm::mat4(1.0f));
				std::pair<bool, glm::vec3> collisionData = sphere.CheckVertexCollision(vertices[x + y * gridRes].pos, modelMatrix);

				if (collisionData.first)
				{
					vertices[x + y * gridRes].pos += (currentPos - preVertices[x + y * gridRes].pos) + glm::normalize(collisionData.second) * dt;
					preVertices[x + y * gridRes].pos = currentPos;
				}
			}
	}

	void Simulate(bool windFlag, float wind, bool dragFlag, float drag, glm::mat4 modelMatrix, float dt)
	{
		for (int step = 0; step < VERLET_STEPS; step++)
		{
			ApplyGravity(dt);

			if (dragFlag)
				AddDrag(drag, dt);

			if (windFlag)
				AddWind(wind, dt);

#ifdef SPHERE_COLLISION
			Collide(modelMatrix, dt);
#endif // SPHERE_COLLISION

			ApplyConstraints(dt);
		}
	}

	void UpdateVertices(float time)
	{
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		//glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_DYNAMIC_DRAW);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(SimpleVertex), vertices.data(), GL_DYNAMIC_DRAW);
	}

	void Render(Shader& shader, glm::mat4 model)
	{
		shader.use();
		shader.setMat4("model", model);

		glBindTexture(GL_TEXTURE_2D, textureId);

		glDisable(GL_CULL_FACE);

		glBindVertexArray(VAO);

		//glDrawArrays(GL_TRIANGLES, 0, vertices.size());
		glDrawElements(GL_TRIANGLES, triIndices.size(), GL_UNSIGNED_INT, 0);
		//glDrawElements(GL_TRIANGLE_FAN, indices.size(), GL_UNSIGNED_INT, 0);

		glBindVertexArray(0);

		glEnable(GL_CULL_FACE);
	}

	inline unsigned int TextureFromFile(const char* path, bool gamma)
	{
		string filename = string(path);

		unsigned int textureID;
		glGenTextures(1, &textureID);

		int width, height, nrComponents;
		unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
		if (data)
		{
			GLenum format;
			if (nrComponents == 1)
				format = GL_RED;
			else if (nrComponents == 3)
				format = GL_RGB;
			else if (nrComponents == 4)
				format = GL_RGBA;

			glBindTexture(GL_TEXTURE_2D, textureID);
			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			stbi_image_free(data);
		}
		else
		{
			std::cout << "Texture failed to load at path: " << path << std::endl;
			stbi_image_free(data);
		}

		return textureID;
	}
};