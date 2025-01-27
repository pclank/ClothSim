#include <ShadowMap.hpp>

ShadowMap::ShadowMap(Shader& shader, Shader& debugShader, const unsigned int width, const unsigned int height,
    const unsigned int screenWidth, const unsigned int screenHeight, const float nearPlane, const float farPlane)
	:
    shader(shader), debugShader(debugShader), width(width), height(height), screenWidth(screenWidth), screenHeight(screenHeight),
    nearPlane(nearPlane), farPlane(farPlane)
{
    glGenFramebuffers(1, &depthMapFBO);

    // Create map
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
        width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    /*glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);*/
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    // Set up framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

ShadowMap::~ShadowMap()
{}

void ShadowMap::Render(float* lightPos, glm::mat4& lightProjection, std::vector<Model>& models, const int nModels, GUI& gui,
    CustomModel* customModel, ClothMesh* clothMesh)
{
    // Render to depth map
    glCullFace(GL_FRONT);
    glViewport(0, 0, width, height);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    shader.use();
    shader.setMat4("lightSpaceMatrix", GetLightSpaceMatrix(lightPos, lightProjection));

    for (size_t i = 0; i < nModels; i++)
        if (gui.modelSets[i].enabled)
            models[i].Draw(shader, gui.modelSets[i].GetModelMatrix());

    if (customModel != nullptr)
    {
        //customModel->Render(shader, gui.customModelSettings.GetModelMatrix());
        customModel->Render(shader, glm::mat4(1.0f));
    }

    // TODO: Add gui control!
    if (clothMesh != nullptr)
        customModel->Render(shader, glm::mat4(1.0f));

    glCullFace(GL_BACK);
}

void ShadowMap::Debug()
{
    debugShader.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    RenderDebugQuad();
}

glm::mat4 ShadowMap::GetLightSpaceMatrix(float* lightPos, glm::mat4& lightProjection)
{
    glm::vec3 lightPosVec(lightPos[0], lightPos[1], lightPos[2]);
    glm::mat4 lightView = glm::lookAt(lightPosVec,
        glm::vec3(0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f));

    return lightProjection * lightView;
}