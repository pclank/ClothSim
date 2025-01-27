#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Shader.hpp>
#include <Model.hpp>
#include <GUI.hpp>
#include <CustomModel.hpp>
#include <ClothMesh.hpp>
#include <vector>

class ShadowCubemap {
public:
	unsigned int width, height, screenWidth, screenHeight;
	unsigned int depthCubemapFBO, depthCubemap;
	float nearPlane, farPlane, aspectRatio;
    glm::mat4 shadowProjectionMatrix;
	Shader shader, debugShader;

	ShadowCubemap(Shader& shader, Shader& debugShader, const unsigned int width, const unsigned int height,
		const unsigned int screenWidth, const unsigned int screenHeight, const float nearPlane = 1.0f, const float farPlane = 25.0f);

	~ShadowCubemap();

	void Render(float* lightPos, std::vector<Model>& models, const int nModels, GUI& gui, CustomModel* customModel = nullptr,
		ClothMesh* clothMesh = nullptr);

    void GetLightSpaceMatrices(float* lightPos, std::vector<glm::mat4>& lightTransformMatrices);
};
