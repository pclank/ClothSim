#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Shader.hpp>
#include <Model.hpp>
#include <GUI.hpp>
#include <CustomModel.hpp>
#include <vector>

static const float debugQuadVertices[] = {
	// positions        // texture Coords
	-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
	-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
	 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
	 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
};

static unsigned int debugQuadVAO = 0;
static unsigned int debugQuadVBO;
inline void RenderDebugQuad()
{
    if (debugQuadVAO == 0)
    {
        // setup plane VAO
        glGenVertexArrays(1, &debugQuadVAO);
        glGenBuffers(1, &debugQuadVBO);
        glBindVertexArray(debugQuadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, debugQuadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(debugQuadVertices), &debugQuadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(debugQuadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

class ShadowMap {
public:
	unsigned int width, height, screenWidth, screenHeight;
	unsigned int depthMapFBO, depthMap;
	float nearPlane, farPlane;
	Shader shader, debugShader;

	ShadowMap(Shader& shader, Shader& debugShader, const unsigned int width, const unsigned int height,
        const unsigned int screenWidth, const unsigned int screenHeight, const float nearPlane = 1.0f, const float farPlane = 7.5f);

	~ShadowMap();

	void Render(float* lightPos, glm::mat4& lightProjection, std::vector<Model>& models, const int nModels, GUI& gui,
        CustomModel* customModel = nullptr);

	void Debug();

    glm::mat4 GetLightSpaceMatrix(float* lightPos, glm::mat4& lightProjection);
};