#pragma once

#include <Shader.hpp>
#include <Sphere.hpp>
#include <ExtraMath.hpp>
#include <PerlinNoise.hpp>
#include <ErrorChecks.hpp>
#include <vector>
#include <array>
#include <direct.h>
#include <glm/glm.hpp>

//#define SPHERE_COLLISION
#define HANDLE_BOTTOM_CORNERS

//#define CROSS_LENGTHS
//#define ANCHORS
//#define PERLIN_NOISE
#define SIMD

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

const float slack = 1.15f;

const int xOffsets[8] = { 1, -1, 0, 0, 1, -1, 1, -1 };
const int yOffsets[8] = { 0, 0, 1, -1, 1,  1, -1, -1 };

struct ClothMesh {
	float width, depth, widthStep, depthStep, dU, dV;
	//std::vector<glm::vec3> vertices, preVertices, fixedVertices;
	std::vector<SimpleVertex> vertices, preVertices, fixedVertices;
	std::vector<glm::vec2> texCoords;
	std::vector<unsigned int> indices, triIndices;
#ifdef CROSS_LENGTHS
	std::vector<std::array<float, 8>> restLengths;				// 8 (except edges) initial distances to neigthbors
#else
	std::vector<std::array<float, 4>> restLengths;				// 4 (except edges) initial distances to neigthbors
#endif // CROSS_LENGTHS
#ifdef ANCHORS
	std::vector<float> anchorLengths;
	std::vector<unsigned int> anchorIndices;
#endif // ANCHORS

#ifdef SIMD
	std::vector<float> VertexPosEven_x, VertexPosOdd_x, VertexPosEven_y, VertexPosOdd_y, VertexPosEven_z, VertexPosOdd_z;	// CurrentPos
	std::vector<float> VertexPrevEven_x, VertexPrevOdd_x, VertexPrevEven_y, VertexPrevOdd_y, VertexPrevEven_z, VertexPrevOdd_z;	// PrevPos
	std::vector<float> VertexFixedEven_x, VertexFixedOdd_x, VertexFixedEven_y, VertexFixedOdd_y, VertexFixedEven_z, VertexFixedOdd_z;	// FixedPoints

	std::vector<std::array<float, 4>> restLengthsEven;
	std::vector<std::array<float, 4>> restLengthsOdd;

	std::array<int, 4> neighborOffsets;
#endif // SIMD


	std::array<float, 2> leftCornerRestLengths, rightCornerRestLengths;
	std::vector<std::array<float, 3>> leftRestLengths, rightRestLengths;
	unsigned int VAO, VBO, EBO;
	unsigned int gridRes;
	unsigned int textureId;
	std::map<unsigned int, unsigned int> restMap;	// Maps vertex coordinates (x + y * gridRes) to restLength indices
	std::map<unsigned int, unsigned int> vertexAnchorMap;	// Maps vertex coordinates (x + y * gridRes) to anchorLength and anchorIndices indices

	ClothMesh(float width, float depth, unsigned int wP, unsigned int dP, unsigned int gridRes,
		std::string textureFile = "clothTexture.jpg", float initHeight = 2.0f)
		:
		width(width), depth(depth), gridRes(gridRes)
	{
#ifdef SIMD
		if (gridRes * gridRes % 16 != 0)
			std::runtime_error("ERROR::AVX is enabled, but size was not a power of 8!");

		neighborOffsets[0] = 1;
		neighborOffsets[1] = -1;
		neighborOffsets[2] = gridRes / 2;
		neighborOffsets[3] = -(gridRes / 2);
#endif // SIMD

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

#ifdef SIMD
		VertexPosEven_x.resize(vertices.size() / 2);
		VertexPosEven_y.resize(vertices.size() / 2);
		VertexPosEven_z.resize(vertices.size() / 2);
		VertexPosOdd_x.resize(vertices.size() / 2);
		VertexPosOdd_y.resize(vertices.size() / 2);
		VertexPosOdd_z.resize(vertices.size() / 2);

		VertexPrevEven_x.resize(vertices.size() / 2);
		VertexPrevEven_y.resize(vertices.size() / 2);
		VertexPrevEven_z.resize(vertices.size() / 2);
		VertexPrevOdd_x.resize(vertices.size() / 2);
		VertexPrevOdd_y.resize(vertices.size() / 2);
		VertexPrevOdd_z.resize(vertices.size() / 2);

		VertexFixedEven_x.resize(gridRes / 2);
		VertexFixedEven_y.resize(gridRes / 2);
		VertexFixedEven_z.resize(gridRes / 2);
		VertexFixedOdd_x.resize(gridRes / 2);
		VertexFixedOdd_y.resize(gridRes / 2);
		VertexFixedOdd_z.resize(gridRes / 2);

		CopyToSIMD(true);
#endif // SIMD

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
		
#ifdef ANCHORS
		anchorLengths.resize(gridRes * (gridRes - 1));
		anchorIndices.resize(gridRes * (gridRes - 1));

		// Calculate anchor distances
		unsigned int vertexIndex = 0;
		for (size_t y = 1; y < gridRes; y++)
			for (size_t x = 0; x < gridRes; x++)
			{
				// Find nearest anchor
				float minLength = 1e30f;
				unsigned int minIndex = 0;

				for (size_t anchor = 0; anchor < gridRes; anchor++)
				{
					float currentLength = glm::length(fixedVertices[anchor].pos - vertices[x + y * gridRes].pos);
					if (currentLength < minLength)
					{
						minLength = currentLength;
						minIndex = anchor;
					}
				}

				// Add to map
				vertexAnchorMap[x + y * gridRes] = vertexIndex;

				anchorLengths[vertexIndex] = minLength;
				anchorIndices[vertexIndex] = minIndex;

				vertexIndex++;
			}

#endif // ANCHORS

		// Calculate rest length
		restLengths.resize(vertices.size() - 4 * gridRes + 4);

		unsigned int restIndex = 0;
		for (size_t y = 1; y < gridRes - 1; y++)
			for (size_t x = 1; x < gridRes - 1; x++)
			{
				// Slack
#ifdef CROSS_LENGTHS
				for (int c = 0; c < 8; c++)
#else
				for (int c = 0; c < 4; c++)
#endif
				{
					restLengths[restIndex][c] = glm::length(vertices[x + y * gridRes].pos -
						vertices[x + xOffsets[c] + (y + yOffsets[c]) * gridRes].pos) * slack;
				}

				// Add to map
				restMap[x + y * gridRes] = restIndex;

				restIndex++;
			}

#ifdef SIMD
		restLengthsEven.resize(gridRes * gridRes /2);
		restLengthsOdd.resize(gridRes * gridRes / 2);

		for (size_t y = 0; y < gridRes; y++)
			for (size_t x = 0; x < gridRes; x++)
			{
				size_t ind = x + y * gridRes;
				bool even = ind % 2 == 0;
				for (int c = 0; c < 4; c++)
				{
					// Right
					if (c == 0 && x == gridRes - 1)
					{
						if (even)
							restLengthsEven[ind / 2][c] = 0.0f;
						else
							restLengthsOdd[(ind - 1) / 2][c] = 0.0f;

						continue;
					}
					// Left
					else if (c == 1 && ind % 2 == 0 && ind % gridRes == 0)
					{
						if (even)
							restLengthsEven[ind / 2][c] = 0.0f;
						else
							restLengthsOdd[(ind - 1) / 2][c] = 0.0f;

						continue;
					}
					// Down
					else if (c == 2 && y == gridRes - 1)
					{
						if (even)
							restLengthsEven[ind / 2][c] = 0.0f;
						else
							restLengthsOdd[(ind - 1) / 2][c] = 0.0f;

						continue;
					}
					// Up
					else if (c == 3 && ind < gridRes)
					{
						if (even)
							restLengthsEven[ind / 2][c] = 0.0f;
						else
							restLengthsOdd[(ind - 1) / 2][c] = 0.0f;

						continue;
					}

					if (even)
						restLengthsEven[ind / 2][c] = glm::length(vertices[x + y * gridRes].pos -
						vertices[x + xOffsets[c] + (y + yOffsets[c]) * gridRes].pos) * slack;
					else
						restLengthsOdd[(ind - 1) / 2][c] = glm::length(vertices[x + y * gridRes].pos -
							vertices[x + xOffsets[c] + (y + yOffsets[c]) * gridRes].pos) * slack;
				}
			}
#endif // SIMD

		// Calculate side vertices rest lengths
		for (unsigned int y = 1; y < gridRes - 1; y++)
		{
			std::array<float, 3> tmpLengths = {
				glm::length(vertices[y * gridRes].pos - vertices[1 + y * gridRes].pos) * slack,					// Right neighbor
				glm::length(vertices[y * gridRes].pos - vertices[(y - 1) * gridRes].pos) * slack,				// Top neighbor
				glm::length(vertices[y * gridRes].pos - vertices[(y + 1) * gridRes].pos) * slack				// Bottom neighbor
			};
			leftRestLengths.push_back(tmpLengths);

			tmpLengths = {
				glm::length(vertices[gridRes - 1 + y * gridRes].pos - vertices[gridRes - 2 + y * gridRes].pos) * slack,			// Left neighbor
				glm::length(vertices[gridRes - 1 + y * gridRes].pos - vertices[gridRes - 1 + (y - 1) * gridRes].pos) * slack,	// Top neighbor
				glm::length(vertices[gridRes - 1 + y * gridRes].pos - vertices[gridRes - 1 + (y + 1) * gridRes].pos) * slack	// Bottom neighbor
			};
			rightRestLengths.push_back(tmpLengths);
		}


		// Calculate bottom corner rest lengths
		leftCornerRestLengths = {
			glm::length(vertices[(gridRes - 1) * gridRes].pos - vertices[1 + (gridRes - 1) * gridRes].pos) * slack,		// Right neighbor
			glm::length(vertices[(gridRes - 1) * gridRes].pos -	vertices[(gridRes - 2) * gridRes].pos) * slack };		// Top neighbor

		rightCornerRestLengths = {
			glm::length(vertices[(gridRes - 1) + (gridRes - 1) * gridRes].pos - vertices[(gridRes - 2) + (gridRes - 1) * gridRes].pos) * slack,		// Left neighbor
			glm::length(vertices[(gridRes - 1) + (gridRes - 1) * gridRes].pos - vertices[(gridRes - 1) + (gridRes - 2) * gridRes].pos) * slack };	// Top neighbor };								// Top neighbor

		std::cout << "Created cloth mesh with " << vertices.size() << " vertices and " << triIndices.size() << " indices" << std::endl;
	}

#ifdef SIMD

	inline void CopyToSIMD(bool first = false)
	{
		for (size_t y = 0; y < gridRes; y++)
			for (size_t x = 0; x < gridRes; x++)
			{
				size_t index = x + y * gridRes;

				// Even vertices
				if (index % 2 == 0)
				{
					VertexPosEven_x[index / 2] = vertices[index].pos.x;
					VertexPosEven_y[index / 2] = vertices[index].pos.y;
					VertexPosEven_z[index / 2] = vertices[index].pos.z;

					VertexPrevEven_x[index / 2] = preVertices[index].pos.x;
					VertexPrevEven_y[index / 2] = preVertices[index].pos.y;
					VertexPrevEven_z[index / 2] = preVertices[index].pos.z;

					// Fixed points
					if (first && y == 0)
					{
						VertexPrevEven_x[index / 2] = vertices[index].pos.x;
						VertexPrevEven_y[index / 2] = vertices[index].pos.y;
						VertexPrevEven_z[index / 2] = vertices[index].pos.z;

						VertexFixedEven_x[index / 2] = vertices[index].pos.x;
						VertexFixedEven_y[index / 2] = vertices[index].pos.y;
						VertexFixedEven_z[index / 2] = vertices[index].pos.z;
					}
				}
				// Odd vertices
				else
				{
					VertexPosOdd_x[(index - 1) / 2] = vertices[index].pos.x;
					VertexPosOdd_y[(index - 1) / 2] = vertices[index].pos.y;
					VertexPosOdd_z[(index - 1) / 2] = vertices[index].pos.z;

					VertexPrevOdd_x[(index - 1) / 2] = preVertices[index].pos.x;
					VertexPrevOdd_y[(index - 1) / 2] = preVertices[index].pos.y;
					VertexPrevOdd_z[(index - 1) / 2] = preVertices[index].pos.z;

					// Fixed points
					if (first && y == 0)
					{
						VertexPrevOdd_x[(index - 1) / 2] = vertices[index].pos.x;
						VertexPrevOdd_y[(index - 1) / 2] = vertices[index].pos.y;
						VertexPrevOdd_z[(index - 1) / 2] = vertices[index].pos.z;

						VertexFixedOdd_x[(index - 1) / 2] = vertices[index].pos.x;
						VertexFixedOdd_y[(index - 1) / 2] = vertices[index].pos.y;
						VertexFixedOdd_z[(index - 1) / 2] = vertices[index].pos.z;
					}
				}
			}
	}

	inline void CopyFromSIMD()
	{
		for (size_t y = 0; y < gridRes; y++)
			for (size_t x = 0; x < gridRes; x++)
			{
				size_t index = x + y * gridRes;

				// Even vertices
				if (index % 2 == 0)
				{
					vertices[index].pos.x = VertexPosEven_x[index / 2];
					vertices[index].pos.y = VertexPosEven_y[index / 2];
					vertices[index].pos.z = VertexPosEven_z[index / 2];

					preVertices[index].pos.x = VertexPrevEven_x[index / 2];
					preVertices[index].pos.y = VertexPrevEven_y[index / 2];
					preVertices[index].pos.z = VertexPrevEven_z[index / 2];
				}
				// Odd vertices
				else
				{
					vertices[index].pos.x = VertexPosOdd_x[(index - 1) / 2];
					vertices[index].pos.y = VertexPosOdd_y[(index - 1) / 2];
					vertices[index].pos.z = VertexPosOdd_z[(index - 1) / 2];

					preVertices[index].pos.x = VertexPrevOdd_x[(index - 1) / 2];
					preVertices[index].pos.y = VertexPrevOdd_y[(index - 1) / 2];
					preVertices[index].pos.z = VertexPrevOdd_z[(index - 1) / 2];
				}
			}
	}

#endif // SIMD

	~ClothMesh()
	{}

	inline void ApplyGravity(float dt)
	{
#ifdef SIMD
		static const __m256 gravity8 = _mm256_set1_ps(-GRAVITY);

		const __m256 dt8 = _mm256_set1_ps(dt);

		// Even points
		float* pos_x = &VertexPosEven_x[gridRes / 2];
		float* pos_y = &VertexPosEven_y[gridRes / 2];
		float* pos_z = &VertexPosEven_z[gridRes / 2];
		float* prevPos_x = &VertexPrevEven_x[gridRes / 2];
		float* prevPos_y = &VertexPrevEven_y[gridRes / 2];
		float* prevPos_z = &VertexPrevEven_z[gridRes / 2];
		float* end = pos_x + (gridRes - 1) * gridRes / 2;

		for (pos_x; pos_x < end; pos_x += 8, prevPos_x += 8, pos_y += 8, prevPos_y += 8, pos_z += 8, prevPos_z += 8)
		{
			__m256 currentPos_x8 = _mm256_load_ps(pos_x);
			__m256 currentPos_y8 = _mm256_load_ps(pos_y);
			__m256 currentPos_z8 = _mm256_load_ps(pos_z);
			__m256 prevPos_x8 = _mm256_load_ps(prevPos_x);
			__m256 prevPos_y8 = _mm256_load_ps(prevPos_y);
			__m256 prevPos_z8 = _mm256_load_ps(prevPos_z);

			__m256 newPos_x8 = _mm256_add_ps(currentPos_x8, _mm256_sub_ps(currentPos_x8, prevPos_x8));
			__m256 newPos_y8 = _mm256_add_ps(currentPos_y8, _mm256_add_ps(_mm256_sub_ps(currentPos_y8, prevPos_y8), _mm256_mul_ps(gravity8, dt8)));
			__m256 newPos_z8 = _mm256_add_ps(currentPos_z8, _mm256_sub_ps(currentPos_z8, prevPos_z8));

			// Store new
			_mm256_store_ps(pos_x, newPos_x8);
			_mm256_store_ps(pos_y, newPos_y8);
			_mm256_store_ps(pos_z, newPos_z8);

			// Update previous
			_mm256_store_ps(prevPos_x, currentPos_x8);
			_mm256_store_ps(prevPos_y, currentPos_y8);
			_mm256_store_ps(prevPos_z, currentPos_z8);
		}

		// Odd points
		pos_x = &VertexPosOdd_x[gridRes / 2];
		pos_y = &VertexPosOdd_y[gridRes / 2];
		pos_z = &VertexPosOdd_z[gridRes / 2];
		prevPos_x = &VertexPrevOdd_x[gridRes / 2];
		prevPos_y = &VertexPrevOdd_y[gridRes / 2];
		prevPos_z = &VertexPrevOdd_z[gridRes / 2];
		end = pos_x + (gridRes - 1) * gridRes / 2;

		for (pos_x; pos_x < end; pos_x += 8, prevPos_x += 8, pos_y += 8, prevPos_y += 8, pos_z += 8, prevPos_z += 8)
		{
			__m256 currentPos_x8 = _mm256_load_ps(pos_x);
			__m256 currentPos_y8 = _mm256_load_ps(pos_y);
			__m256 currentPos_z8 = _mm256_load_ps(pos_z);
			__m256 prevPos_x8 = _mm256_load_ps(prevPos_x);
			__m256 prevPos_y8 = _mm256_load_ps(prevPos_y);
			__m256 prevPos_z8 = _mm256_load_ps(prevPos_z);

			__m256 newPos_x8 = _mm256_add_ps(currentPos_x8, _mm256_sub_ps(currentPos_x8, prevPos_x8));
			__m256 newPos_y8 = _mm256_add_ps(currentPos_y8, _mm256_add_ps(_mm256_sub_ps(currentPos_y8, prevPos_y8), _mm256_mul_ps(gravity8, dt8)));
			__m256 newPos_z8 = _mm256_add_ps(currentPos_z8, _mm256_sub_ps(currentPos_z8, prevPos_z8));

			// Store new
			_mm256_store_ps(pos_x, newPos_x8);
			_mm256_store_ps(pos_y, newPos_y8);
			_mm256_store_ps(pos_z, newPos_z8);

			// Update previous
			_mm256_store_ps(prevPos_x, currentPos_x8);
			_mm256_store_ps(prevPos_y, currentPos_y8);
			_mm256_store_ps(prevPos_z, currentPos_z8);
		}

		return;

#endif // SIMD

		for (size_t y = 1; y < gridRes; y++)
			for (size_t x = 0; x < gridRes; x++)
			{
				const glm::vec3 currentPos = vertices[x + y * gridRes].pos;
				const glm::vec3 prevPos = preVertices[x + y * gridRes].pos;

				vertices[x + y * gridRes].pos += (currentPos - prevPos) + gravity * dt;
				//vertices[x + y * gridRes].pos += (currentPos - prevPos) + gravity;

#ifdef ANCHORS
				unsigned int anchorIndex = vertexAnchorMap.at(x + y * gridRes);

				const float currentLength = glm::length(fixedVertices[anchorIndices[anchorIndex]].pos - vertices[x + y * gridRes].pos);
				if (currentLength > anchorLengths[anchorIndex])
				{
					// Clamp to proper distance from anchor
					const float delta = fabs(currentLength - anchorLengths[anchorIndex]);

					const glm::vec3 distanceVector = glm::normalize(fixedVertices[anchorIndices[anchorIndex]].pos - vertices[x + y * gridRes].pos);
					vertices[x + y * gridRes].pos += distanceVector * delta;
				}
#endif // ANCHORS

				if (!isfinite(glm::length(vertices[x + y * gridRes].pos)))
				{
					//throw std::runtime_error("gravity issue");
					vertices[x + y * gridRes].pos = preVertices[x + y * gridRes].pos;
					continue;
				}

				preVertices[x + y * gridRes].pos = currentPos;

				// if (Rand( 10 ) < 0.03f) grid( x, y ).pos += float2( Rand( 0.02f + magic ), Rand( 0.12f ) );
			}
	}

	inline void ApplyConstraints(float dt)
	{
#ifdef SIMD
		static const __m256 one8 = _mm256_set1_ps(1.0f);
		static const __m256 zero8 = _mm256_set1_ps(0.0f);
		static const __m256 pointFive8 = _mm256_set1_ps(0.5f);
		static const __m256 rightUpdateMask = _mm256_setr_ps(0, 1, 1, 1, 1, 1, 1, 1);
		static const __m256 leftUpdateMask = _mm256_setr_ps(1, 1, 1, 1, 1, 1, 1, 0);

		const __m256 dt8 = _mm256_set1_ps(dt);
#endif // SIMD

		for (int i = 0; i < CONSTRAINT_STEPS; i++)
		{

#ifdef SIMD
			//CopyToSIMD();

			// TODO: Pointer should be ignoring the first row of vertices (they are fixed)!

			uint32_t pointIndex = gridRes / 2;	// Skips first row

			// Even points
			float* pos_x = &VertexPosEven_x[pointIndex];
			float* pos_y = &VertexPosEven_y[pointIndex];
			float* pos_z = &VertexPosEven_z[pointIndex];
			//int endIndex = VertexPosEven_x.size() - (gridRes/2) - 8;		// Skips final row
			int endIndex = VertexPosEven_x.size() - 8;						// Includes final row
			float* end = &VertexPosEven_x[endIndex];

			for (pos_x; pos_x <= end; pos_x += 8, pos_y += 8, pos_z += 8, pointIndex += 8)
			{
				__m256 currentPos_x8 = _mm256_load_ps(pos_x);
				__m256 currentPos_y8 = _mm256_load_ps(pos_y);
				__m256 currentPos_z8 = _mm256_load_ps(pos_z);

				// Update for all neighbors
				for (int linknr = 0; linknr < 4; linknr++)
				{
					float* neighbor_x;
					float* neighbor_y;
					float* neighbor_z;

					// Final row skip down neighbor
					if (pointIndex > VertexPosEven_x.size() - (gridRes / 2) - 8 && linknr == 2)
					{
						continue;
					}

					// Right Neighbor
					if (linknr == 0)
					{
						neighbor_x = &VertexPosOdd_x[pointIndex];
						neighbor_y = &VertexPosOdd_y[pointIndex];
						neighbor_z = &VertexPosOdd_z[pointIndex];
					}
					// Left Neighbor
					else if (linknr == 1)
					{
						neighbor_x = &VertexPosOdd_x[pointIndex - 1];
						neighbor_y = &VertexPosOdd_y[pointIndex - 1];
						neighbor_z = &VertexPosOdd_z[pointIndex - 1];
					}
					// Up/Down Neighbors
					else
					{
						neighbor_x = pos_x + neighborOffsets[linknr];
						neighbor_y = pos_y + neighborOffsets[linknr];
						neighbor_z = pos_z + neighborOffsets[linknr];
					}

					// Calculate distance
					__m256 neighbor_x8 = _mm256_load_ps(neighbor_x);
					__m256 neighbor_y8 = _mm256_load_ps(neighbor_y);
					__m256 neighbor_z8 = _mm256_load_ps(neighbor_z);

					__m256 dir_x8 = _mm256_sub_ps(neighbor_x8, currentPos_x8);
					__m256 dir_y8 = _mm256_sub_ps(neighbor_y8, currentPos_y8);
					__m256 dir_z8 = _mm256_sub_ps(neighbor_z8, currentPos_z8);

					__m256 distance8 = _mm256_sqrt_ps(_mm256_add_ps(_mm256_mul_ps(dir_x8, dir_x8), _mm256_add_ps(_mm256_mul_ps(dir_y8, dir_y8), _mm256_mul_ps(dir_z8, dir_z8))));

					// Retrieve rest lengths
					__m256 restLengths8 = _mm256_setr_ps(
						restLengthsEven[pointIndex][linknr], restLengthsEven[pointIndex + 1][linknr],
						restLengthsEven[pointIndex + 2][linknr], restLengthsEven[pointIndex + 3][linknr],
						restLengthsEven[pointIndex + 4][linknr], restLengthsEven[pointIndex + 5][linknr],
						restLengthsEven[pointIndex + 6][linknr], restLengthsEven[pointIndex + 7][linknr]
					);

					__m256 distanceMask = _mm256_cmp_ps(distance8, restLengths8, _CMP_GT_OQ);

					__m256 force8 = _mm256_sub_ps(_mm256_div_ps(distance8, restLengths8), one8);
					__m256 forcePointFiveDt8 = _mm256_mul_ps(force8, _mm256_mul_ps(pointFive8, dt8));

					// Use mask
					forcePointFiveDt8 = _mm256_and_ps(forcePointFiveDt8, distanceMask);

					// Start of row
					if (pointIndex % (gridRes / 2) == 0 && linknr == 1)
					{
						forcePointFiveDt8 = _mm256_and_ps(forcePointFiveDt8, rightUpdateMask);
					}

					// Update current positions
					currentPos_x8 = _mm256_fmadd_ps(dir_x8, forcePointFiveDt8, currentPos_x8);
					currentPos_y8 = _mm256_fmadd_ps(dir_y8, forcePointFiveDt8, currentPos_y8);
					currentPos_z8 = _mm256_fmadd_ps(dir_z8, forcePointFiveDt8, currentPos_z8);

					IsInfSIMD(currentPos_x8);
					IsInfSIMD(currentPos_y8);
					IsInfSIMD(currentPos_z8);
					IsNaNSIMD(currentPos_x8);
					IsNaNSIMD(currentPos_y8);
					IsNaNSIMD(currentPos_z8);

					// Update neighbors
					_mm256_store_ps(neighbor_x, _mm256_fnmadd_ps(dir_x8, forcePointFiveDt8, neighbor_x8));
					_mm256_store_ps(neighbor_y, _mm256_fnmadd_ps(dir_y8, forcePointFiveDt8, neighbor_y8));
					_mm256_store_ps(neighbor_z, _mm256_fnmadd_ps(dir_z8, forcePointFiveDt8, neighbor_z8));
				}

				// Update positions
				_mm256_store_ps(pos_x, currentPos_x8);
				_mm256_store_ps(pos_y, currentPos_y8);
				_mm256_store_ps(pos_z, currentPos_z8);
			}

			// Odd points
			pointIndex = gridRes / 2;
			pos_x = &VertexPosOdd_x[pointIndex];
			pos_y = &VertexPosOdd_y[pointIndex];
			pos_z = &VertexPosOdd_z[pointIndex];
			end = &VertexPosOdd_x[endIndex];

			int prevEndIndex = ((gridRes / 2) - 8);

			for (pos_x; pos_x <= end; pos_x += 8, pos_y += 8, pos_z += 8, pointIndex += 8)
			{
				__m256 currentPos_x8 = _mm256_load_ps(pos_x);
				__m256 currentPos_y8 = _mm256_load_ps(pos_y);
				__m256 currentPos_z8 = _mm256_load_ps(pos_z);

				// Update for all neighbors
				for (int linknr = 0; linknr < 4; linknr++)
				{
					float* neighbor_x;
					float* neighbor_y;
					float* neighbor_z;

					// Final row skip down neighbor
					// TODO: May be incorrect!
					if (pointIndex > VertexPosOdd_x.size() - (gridRes / 2) - 8 && linknr == 2)
					{
						continue;
					}

					// Right Neighbor
					if (linknr == 0)
					{
						neighbor_x = &VertexPosEven_x[pointIndex + 1];
						neighbor_y = &VertexPosEven_y[pointIndex + 1];
						neighbor_z = &VertexPosEven_z[pointIndex + 1];
					}
					// Left Neighbor
					else if (linknr == 1)
					{
						neighbor_x = &VertexPosEven_x[pointIndex];
						neighbor_y = &VertexPosEven_y[pointIndex];
						neighbor_z = &VertexPosEven_z[pointIndex];
					}
					// Up/Down Neighbors
					else
					{
						neighbor_x = pos_x + neighborOffsets[linknr];
						neighbor_y = pos_y + neighborOffsets[linknr];
						neighbor_z = pos_z + neighborOffsets[linknr];
					}

					// Calculate distance
					__m256 neighbor_x8 = _mm256_load_ps(neighbor_x);
					__m256 neighbor_y8 = _mm256_load_ps(neighbor_y);
					__m256 neighbor_z8 = _mm256_load_ps(neighbor_z);

					__m256 dir_x8 = _mm256_sub_ps(neighbor_x8, currentPos_x8);
					__m256 dir_y8 = _mm256_sub_ps(neighbor_y8, currentPos_y8);
					__m256 dir_z8 = _mm256_sub_ps(neighbor_z8, currentPos_z8);

					__m256 distance8 = _mm256_sqrt_ps(_mm256_add_ps(_mm256_mul_ps(dir_x8, dir_x8), _mm256_add_ps(_mm256_mul_ps(dir_y8, dir_y8), _mm256_mul_ps(dir_z8, dir_z8))));

					// Retrieve rest lengths
					__m256 restLengths8 = _mm256_setr_ps(
						restLengthsOdd[pointIndex][linknr], restLengthsOdd[pointIndex + 1][linknr],
						restLengthsOdd[pointIndex + 2][linknr], restLengthsOdd[pointIndex + 3][linknr],
						restLengthsOdd[pointIndex + 4][linknr], restLengthsOdd[pointIndex + 5][linknr],
						restLengthsOdd[pointIndex + 6][linknr], restLengthsOdd[pointIndex + 7][linknr]
					);

					__m256 distanceMask = _mm256_cmp_ps(distance8, restLengths8, _CMP_GT_OQ);

					__m256 force8 = _mm256_sub_ps(_mm256_div_ps(distance8, restLengths8), one8);
					__m256 forcePointFiveDt8 = _mm256_mul_ps(force8, _mm256_mul_ps(pointFive8, dt8));

					// Use mask
					forcePointFiveDt8 = _mm256_and_ps(forcePointFiveDt8, distanceMask);

					// End of row
					//if ((gridRes / 2 == 8 || pointIndex % ((gridRes / 2) - 8) == 0) && linknr == 0)
					if ((gridRes / 2 == 8 || pointIndex - prevEndIndex == (gridRes / 2)) && linknr == 0)
					{
						forcePointFiveDt8 = _mm256_and_ps(forcePointFiveDt8, leftUpdateMask);
						prevEndIndex = pointIndex;
					}

					// Update current positions
					currentPos_x8 = _mm256_fmadd_ps(dir_x8, forcePointFiveDt8, currentPos_x8);
					currentPos_y8 = _mm256_fmadd_ps(dir_y8, forcePointFiveDt8, currentPos_y8);
					currentPos_z8 = _mm256_fmadd_ps(dir_z8, forcePointFiveDt8, currentPos_z8);

					IsInfSIMD(currentPos_x8);
					IsInfSIMD(currentPos_y8);
					IsInfSIMD(currentPos_z8);
					IsNaNSIMD(currentPos_x8);
					IsNaNSIMD(currentPos_y8);
					IsNaNSIMD(currentPos_z8);

					// Update neighbors
					_mm256_store_ps(neighbor_x, _mm256_fnmadd_ps(dir_x8, forcePointFiveDt8, neighbor_x8));
					_mm256_store_ps(neighbor_y, _mm256_fnmadd_ps(dir_y8, forcePointFiveDt8, neighbor_y8));
					_mm256_store_ps(neighbor_z, _mm256_fnmadd_ps(dir_z8, forcePointFiveDt8, neighbor_z8));
				}

				// Update positions
				_mm256_store_ps(pos_x, currentPos_x8);
				_mm256_store_ps(pos_y, currentPos_y8);
				_mm256_store_ps(pos_z, currentPos_z8);
			}

			//CopyFromSIMD();

			// Fixed even points
			pos_x = VertexPosEven_x.data();
			pos_y = VertexPosEven_y.data();
			pos_z = VertexPosEven_z.data();
			float* fixedPos_x = VertexFixedEven_x.data();
			float* fixedPos_y = VertexFixedEven_y.data();
			float* fixedPos_z = VertexFixedEven_z.data();
			endIndex = VertexFixedEven_x.size() - 8;
			end = &VertexFixedEven_x[endIndex];

			for (pos_x; fixedPos_x <= end; pos_x += 8, pos_y += 8, pos_z += 8, fixedPos_x += 8, fixedPos_y += 8, fixedPos_z += 8)
			{
				_mm256_store_ps(pos_x, _mm256_load_ps(fixedPos_x));
				_mm256_store_ps(pos_y, _mm256_load_ps(fixedPos_y));
				_mm256_store_ps(pos_z, _mm256_load_ps(fixedPos_z));
			}

			// Fixed odd points
			pos_x = VertexPosOdd_x.data();
			pos_y = VertexPosOdd_y.data();
			pos_z = VertexPosOdd_z.data();
			fixedPos_x = VertexFixedOdd_x.data();
			fixedPos_y = VertexFixedOdd_y.data();
			fixedPos_z = VertexFixedOdd_z.data();
			endIndex = VertexFixedOdd_x.size() - 8;
			end = &VertexFixedOdd_x[endIndex];

			for (pos_x; fixedPos_x <= end; pos_x += 8, pos_y += 8, pos_z += 8, fixedPos_x += 8, fixedPos_y += 8, fixedPos_z += 8)
			{
				_mm256_store_ps(pos_x, _mm256_load_ps(fixedPos_x));
				_mm256_store_ps(pos_y, _mm256_load_ps(fixedPos_y));
				_mm256_store_ps(pos_z, _mm256_load_ps(fixedPos_z));
			}
			
			return;
#endif // SIMD

			for (int y = 1; y < gridRes - 1; y++)
				for (int x = 1; x < gridRes - 1; x++)
				{
					glm::vec3 pos = vertices[x + y * gridRes].pos;

					// Use springs constraint vertices
#ifdef CROSS_LENGTHS
					for (int linknr = 0; linknr < 8; linknr++)
#else
					for (int linknr = 0; linknr < 4; linknr++)
#endif // CROSS_LENGTHS
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
							//glm::vec3 impulse = force * direction * 0.5f;
							glm::vec3 impulse = force * direction * 0.5f * dt;
							pos += impulse;
							neighbor -= impulse;
							/*pos -= force * direction * 0.5f;
							neighbor += force * direction * 0.5f;*/
						}

						if (isnan(pos.x) || isnan(pos.y) || isnan(pos.z))
						{
							// TODO: CLAMP!!!
							vertices[x + y * gridRes].pos = preVertices[x + y * gridRes].pos;
							continue;
						}

						vertices[x + y * gridRes].pos = pos;
						vertices[neighborIndex].pos = neighbor;
					}
				}

#ifdef HANDLE_BOTTOM_CORNERS
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
					//glm::vec3 impulse = force * direction * 0.5f;
					glm::vec3 impulse = force * direction * 0.5f * dt;
					leftPos += impulse;
					neighbor -= impulse;
					/*leftPos -= force * direction * 0.5f;
					neighbor += force * direction * 0.5f;*/
				}

				if (isnan(leftPos.x) || isnan(leftPos.y) || isnan(leftPos.z))
				{
					// TODO: CLAMP!!!
					vertices[(gridRes - 1) * gridRes].pos = preVertices[(gridRes - 1) * gridRes].pos;
					continue;
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
					//glm::vec3 impulse = force * direction * 0.5f;
					glm::vec3 impulse = force * direction * 0.5f * dt;
					rightPos += impulse;
					neighbor -= impulse;
					/*rightPos -= force * direction * 0.5f;
					neighbor += force * direction * 0.5f;*/
				}

				if (isnan(rightPos.x) || isnan(rightPos.y) || isnan(rightPos.z))
				{
					// TODO: CLAMP!!!
					vertices[(gridRes - 1) + (gridRes - 1) * gridRes].pos = preVertices[(gridRes - 1) + (gridRes - 1) * gridRes].pos;
					continue;
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
						//glm::vec3 impulse = force * direction * 0.5f;
						glm::vec3 impulse = force * direction * 0.5f * dt;
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
						//glm::vec3 impulse = force * direction * 0.5f;
						glm::vec3 impulse = force * direction * 0.5f * dt;
						pos += impulse;
						neighbor -= impulse;
						/*pos -= force * direction * 0.5f;
						neighbor += force * direction * 0.5f;*/
					}

					vertices[gridRes - 1 + y * gridRes].pos = pos;
					vertices[neighborIndex].pos = neighbor;
				}
			}
#endif // HANDLE_BOTTOM_CORNERS

#ifdef ANCHORS
			for (int y = 1; y < gridRes; y++)
				for (int x = 0; x < gridRes; x++)
				{
					unsigned int anchorIndex = vertexAnchorMap.at(x + y * gridRes);

					const float currentLength = glm::length(fixedVertices[anchorIndices[anchorIndex]].pos - vertices[x + y * gridRes].pos);
					if (currentLength > anchorLengths[anchorIndex])
					{
						// Clamp to proper distance from anchor
						const float delta = fabs(currentLength - anchorLengths[anchorIndex]);

						const glm::vec3 distanceVector = glm::normalize(fixedVertices[anchorIndices[anchorIndex]].pos - vertices[x + y * gridRes].pos);
						vertices[x + y * gridRes].pos += distanceVector * delta;
					}
				}
#endif // ANCHORS

			// Fixed vertices
			for (int x = 0; x < gridRes; x++)
				vertices[x].pos = fixedVertices[x].pos;
		}

		//std::cout << vertices[(gridRes - 1) * gridRes].x << " | " << vertices[(gridRes - 1) * gridRes].y << " | " << vertices[(gridRes - 1) * gridRes].z << std::endl;
	}

	void AddDrag(float drag, float dt)
	{
#ifdef SIMD
		const __m256 drag8 = _mm256_set1_ps(drag);

		const __m256 dt8 = _mm256_set1_ps(dt);

		// Even points
		float* pos_x = &VertexPosEven_x[gridRes / 2];
		float* pos_y = &VertexPosEven_y[gridRes / 2];
		float* pos_z = &VertexPosEven_z[gridRes / 2];
		float* prevPos_x = &VertexPrevEven_x[gridRes / 2];
		float* prevPos_y = &VertexPrevEven_y[gridRes / 2];
		float* prevPos_z = &VertexPrevEven_z[gridRes / 2];
		float* end = pos_x + (gridRes - 1) * gridRes / 2;

		for (pos_x; pos_x < end; pos_x += 8, prevPos_x += 8, pos_y += 8, prevPos_y += 8, pos_z += 8, prevPos_z += 8)
		{
			__m256 currentPos_x8 = _mm256_load_ps(pos_x);
			__m256 currentPos_y8 = _mm256_load_ps(pos_y);
			__m256 currentPos_z8 = _mm256_load_ps(pos_z);
			__m256 prevPos_x8 = _mm256_load_ps(prevPos_x);
			__m256 prevPos_y8 = _mm256_load_ps(prevPos_y);
			__m256 prevPos_z8 = _mm256_load_ps(prevPos_z);

			// Calculate drag directions
			__m256 dragDirection_x8 = _mm256_sub_ps(prevPos_x8, currentPos_x8);
			__m256 dragDirection_y8 = _mm256_sub_ps(prevPos_y8, currentPos_y8);
			__m256 dragDirection_z8 = _mm256_sub_ps(prevPos_z8, currentPos_z8);

			// Apply drag
			__m256 newPos_x8 = _mm256_add_ps(currentPos_x8, _mm256_add_ps(_mm256_sub_ps(currentPos_x8, prevPos_x8), _mm256_mul_ps(dragDirection_x8, _mm256_mul_ps(drag8, dt8))));
			__m256 newPos_y8 = _mm256_add_ps(currentPos_y8, _mm256_add_ps(_mm256_sub_ps(currentPos_y8, prevPos_y8), _mm256_mul_ps(dragDirection_y8, _mm256_mul_ps(drag8, dt8))));
			__m256 newPos_z8 = _mm256_add_ps(currentPos_z8, _mm256_add_ps(_mm256_sub_ps(currentPos_z8, prevPos_z8), _mm256_mul_ps(dragDirection_z8, _mm256_mul_ps(drag8, dt8))));

			// Store new
			_mm256_store_ps(pos_x, newPos_x8);
			_mm256_store_ps(pos_y, newPos_y8);
			_mm256_store_ps(pos_z, newPos_z8);

			// Update previous
			_mm256_store_ps(prevPos_x, currentPos_x8);
			_mm256_store_ps(prevPos_y, currentPos_y8);
			_mm256_store_ps(prevPos_z, currentPos_z8);
		}

		// Odd points
		pos_x = &VertexPosOdd_x[gridRes / 2];
		pos_y = &VertexPosOdd_y[gridRes / 2];
		pos_z = &VertexPosOdd_z[gridRes / 2];
		prevPos_x = &VertexPrevOdd_x[gridRes / 2];
		prevPos_y = &VertexPrevOdd_y[gridRes / 2];
		prevPos_z = &VertexPrevOdd_z[gridRes / 2];
		end = pos_x + (gridRes - 1) * gridRes / 2;

		for (pos_x; pos_x < end; pos_x += 8, prevPos_x += 8, pos_y += 8, prevPos_y += 8, pos_z += 8, prevPos_z += 8)
		{
			__m256 currentPos_x8 = _mm256_load_ps(pos_x);
			__m256 currentPos_y8 = _mm256_load_ps(pos_y);
			__m256 currentPos_z8 = _mm256_load_ps(pos_z);
			__m256 prevPos_x8 = _mm256_load_ps(prevPos_x);
			__m256 prevPos_y8 = _mm256_load_ps(prevPos_y);
			__m256 prevPos_z8 = _mm256_load_ps(prevPos_z);

			// Calculate drag directions
			__m256 dragDirection_x8 = _mm256_sub_ps(prevPos_x8, currentPos_x8);
			__m256 dragDirection_y8 = _mm256_sub_ps(prevPos_y8, currentPos_y8);
			__m256 dragDirection_z8 = _mm256_sub_ps(prevPos_z8, currentPos_z8);

			// Apply drag
			__m256 newPos_x8 = _mm256_add_ps(currentPos_x8, _mm256_add_ps(_mm256_sub_ps(currentPos_x8, prevPos_x8), _mm256_mul_ps(dragDirection_x8, _mm256_mul_ps(drag8, dt8))));
			__m256 newPos_y8 = _mm256_add_ps(currentPos_y8, _mm256_add_ps(_mm256_sub_ps(currentPos_y8, prevPos_y8), _mm256_mul_ps(dragDirection_y8, _mm256_mul_ps(drag8, dt8))));
			__m256 newPos_z8 = _mm256_add_ps(currentPos_z8, _mm256_add_ps(_mm256_sub_ps(currentPos_z8, prevPos_z8), _mm256_mul_ps(dragDirection_z8, _mm256_mul_ps(drag8, dt8))));

			// Store new
			_mm256_store_ps(pos_x, newPos_x8);
			_mm256_store_ps(pos_y, newPos_y8);
			_mm256_store_ps(pos_z, newPos_z8);

			// Update previous
			_mm256_store_ps(prevPos_x, currentPos_x8);
			_mm256_store_ps(prevPos_y, currentPos_y8);
			_mm256_store_ps(prevPos_z, currentPos_z8);
		}

		return;
#endif // SIMD

		for (size_t y = 1; y < gridRes; y++)
			for (size_t x = 0; x < gridRes; x++)
			{
				const glm::vec3 currentPos = vertices[x + y * gridRes].pos;
				const glm::vec3 prevPos = preVertices[x + y * gridRes].pos;

				const glm::vec3 dragDirection = prevPos - currentPos;

				vertices[x + y * gridRes].pos += (currentPos - prevPos) + dragDirection * drag * dt;
				//vertices[x + y * gridRes].pos += (currentPos - prevPos) + dragDirection * drag;

#ifdef ANCHORS
				unsigned int anchorIndex = vertexAnchorMap.at(x + y * gridRes);

				const float currentLength = glm::length(fixedVertices[anchorIndices[anchorIndex]].pos - vertices[x + y * gridRes].pos);
				if (currentLength > anchorLengths[anchorIndex])
				{
					// Clamp to proper distance from anchor
					const float delta = fabs(currentLength - anchorLengths[anchorIndex]);

					const glm::vec3 distanceVector = glm::normalize(fixedVertices[anchorIndices[anchorIndex]].pos - vertices[x + y * gridRes].pos);
					vertices[x + y * gridRes].pos += distanceVector * delta;
				}
#endif // ANCHORS

				if (!isfinite(glm::length(vertices[x + y * gridRes].pos)))
				{
					throw std::runtime_error("drag issue");
				}

				preVertices[x + y * gridRes].pos = currentPos;
			}
	}

	void AddWind(float wind, float dt, bool useManualDirection = false, glm::vec3 manualDirection = glm::vec3(0.0f))
	{
		const glm::vec3 windDirection = (useManualDirection) ? glm::normalize(manualDirection) : glm::normalize(Random3f(-1.0f, 1.0f));

#ifdef SIMD
		const __m256 wind8 = _mm256_set1_ps(wind);

		const __m256 dt8 = _mm256_set1_ps(dt);

		// Even points
		float* pos_x = &VertexPosEven_x[gridRes / 2];
		float* pos_y = &VertexPosEven_y[gridRes / 2];
		float* pos_z = &VertexPosEven_z[gridRes / 2];
		float* prevPos_x = &VertexPrevEven_x[gridRes / 2];
		float* prevPos_y = &VertexPrevEven_y[gridRes / 2];
		float* prevPos_z = &VertexPrevEven_z[gridRes / 2];
		float* end = pos_x + (gridRes - 1) * gridRes / 2;

		for (pos_x; pos_x < end; pos_x += 8, prevPos_x += 8, pos_y += 8, prevPos_y += 8, pos_z += 8, prevPos_z += 8)
		{
			__m256 currentPos_x8 = _mm256_load_ps(pos_x);
			__m256 currentPos_y8 = _mm256_load_ps(pos_y);
			__m256 currentPos_z8 = _mm256_load_ps(pos_z);
			__m256 prevPos_x8 = _mm256_load_ps(prevPos_x);
			__m256 prevPos_y8 = _mm256_load_ps(prevPos_y);
			__m256 prevPos_z8 = _mm256_load_ps(prevPos_z);

			// Wind directions
			__m256 windDirection_x8 = _mm256_set1_ps(windDirection.x);
			__m256 windDirection_y8 = _mm256_set1_ps(windDirection.y);
			__m256 windDirection_z8 = _mm256_set1_ps(windDirection.z);

			// Apply wind
			__m256 newPos_x8 = _mm256_add_ps(currentPos_x8, _mm256_add_ps(_mm256_sub_ps(currentPos_x8, prevPos_x8), _mm256_mul_ps(windDirection_x8, _mm256_mul_ps(wind8, dt8))));
			__m256 newPos_y8 = _mm256_add_ps(currentPos_y8, _mm256_add_ps(_mm256_sub_ps(currentPos_y8, prevPos_y8), _mm256_mul_ps(windDirection_y8, _mm256_mul_ps(wind8, dt8))));
			__m256 newPos_z8 = _mm256_add_ps(currentPos_z8, _mm256_add_ps(_mm256_sub_ps(currentPos_z8, prevPos_z8), _mm256_mul_ps(windDirection_z8, _mm256_mul_ps(wind8, dt8))));

			// Store new
			_mm256_store_ps(pos_x, newPos_x8);
			_mm256_store_ps(pos_y, newPos_y8);
			_mm256_store_ps(pos_z, newPos_z8);

			// Update previous
			_mm256_store_ps(prevPos_x, currentPos_x8);
			_mm256_store_ps(prevPos_y, currentPos_y8);
			_mm256_store_ps(prevPos_z, currentPos_z8);
		}

		// Odd points
		pos_x = &VertexPosOdd_x[gridRes / 2];
		pos_y = &VertexPosOdd_y[gridRes / 2];
		pos_z = &VertexPosOdd_z[gridRes / 2];
		prevPos_x = &VertexPrevOdd_x[gridRes / 2];
		prevPos_y = &VertexPrevOdd_y[gridRes / 2];
		prevPos_z = &VertexPrevOdd_z[gridRes / 2];
		end = pos_x + (gridRes - 1) * gridRes / 2;

		for (pos_x; pos_x < end; pos_x += 8, prevPos_x += 8, pos_y += 8, prevPos_y += 8, pos_z += 8, prevPos_z += 8)
		{
			__m256 currentPos_x8 = _mm256_load_ps(pos_x);
			__m256 currentPos_y8 = _mm256_load_ps(pos_y);
			__m256 currentPos_z8 = _mm256_load_ps(pos_z);
			__m256 prevPos_x8 = _mm256_load_ps(prevPos_x);
			__m256 prevPos_y8 = _mm256_load_ps(prevPos_y);
			__m256 prevPos_z8 = _mm256_load_ps(prevPos_z);

			// Wind directions
			__m256 windDirection_x8 = _mm256_set1_ps(windDirection.x);
			__m256 windDirection_y8 = _mm256_set1_ps(windDirection.y);
			__m256 windDirection_z8 = _mm256_set1_ps(windDirection.z);

			// Apply wind
			__m256 newPos_x8 = _mm256_add_ps(currentPos_x8, _mm256_add_ps(_mm256_sub_ps(currentPos_x8, prevPos_x8), _mm256_mul_ps(windDirection_x8, _mm256_mul_ps(wind8, dt8))));
			__m256 newPos_y8 = _mm256_add_ps(currentPos_y8, _mm256_add_ps(_mm256_sub_ps(currentPos_y8, prevPos_y8), _mm256_mul_ps(windDirection_y8, _mm256_mul_ps(wind8, dt8))));
			__m256 newPos_z8 = _mm256_add_ps(currentPos_z8, _mm256_add_ps(_mm256_sub_ps(currentPos_z8, prevPos_z8), _mm256_mul_ps(windDirection_z8, _mm256_mul_ps(wind8, dt8))));

			// Store new
			_mm256_store_ps(pos_x, newPos_x8);
			_mm256_store_ps(pos_y, newPos_y8);
			_mm256_store_ps(pos_z, newPos_z8);

			// Update previous
			_mm256_store_ps(prevPos_x, currentPos_x8);
			_mm256_store_ps(prevPos_y, currentPos_y8);
			_mm256_store_ps(prevPos_z, currentPos_z8);
		}

		// TODO: Should be moved to Simulate(), so that it only happens once
		//CopyFromSIMD();

		return;
#endif // SIMD

		for (size_t y = 1; y < gridRes; y++)
			for (size_t x = 0; x < gridRes; x++)
			{
				const glm::vec3 currentPos = vertices[x + y * gridRes].pos;
				const glm::vec3 prevPos = preVertices[x + y * gridRes].pos;

				// Perlin Noise test
				//std::cout << PerlinNoise(currentPos.x, currentPos.y, 8) << std::endl;

#ifdef PERLIN_NOISE
				vertices[x + y * gridRes].pos += (currentPos - prevPos) + windDirection * wind * dt * PerlinNoise(currentPos.x, currentPos.y, 8);
#else
				vertices[x + y * gridRes].pos += (currentPos - prevPos) + windDirection * wind * dt;
#endif // PERLIN_NOISE
#ifdef ANCHORS
				unsigned int anchorIndex = vertexAnchorMap.at(x + y * gridRes);

				const float currentLength = glm::length(fixedVertices[anchorIndices[anchorIndex]].pos - vertices[x + y * gridRes].pos);
				if (currentLength > anchorLengths[anchorIndex])
				{
					// Clamp to proper distance from anchor
					const float delta = fabs(currentLength - anchorLengths[anchorIndex]);

					const glm::vec3 distanceVector = glm::normalize(fixedVertices[anchorIndices[anchorIndex]].pos - vertices[x + y * gridRes].pos);
					vertices[x + y * gridRes].pos += distanceVector * delta;
				}
#endif // ANCHORS

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

	void Simulate(bool windFlag, float wind, bool dragFlag, float drag, bool useManualWind, glm::vec3 manualWindDirection, glm::mat4 modelMatrix, float dt)
	{
#ifdef SIMD
		CopyToSIMD();
#endif // SIMD

		for (int step = 0; step < VERLET_STEPS; step++)
		{
			ApplyGravity(dt);

			if (windFlag)
				AddWind(wind, dt, useManualWind, manualWindDirection);

			if (dragFlag)
				AddDrag(drag, dt);

#ifdef SPHERE_COLLISION
			Collide(modelMatrix, dt);
#endif // SPHERE_COLLISION

			ApplyConstraints(dt);
		}

#ifdef SIMD
		CopyFromSIMD();
#endif // SIMD
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