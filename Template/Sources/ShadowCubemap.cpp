#include <ShadowCubemap.hpp>

ShadowCubemap::ShadowCubemap(Shader& shader, Shader& debugShader, const unsigned int width, const unsigned int height,
    const unsigned int screenWidth, const unsigned int screenHeight, const float nearPlane, const float farPlane)
    :
    shader(shader), debugShader(debugShader), width(width), height(height), screenWidth(screenWidth), screenHeight(screenHeight),
    nearPlane(nearPlane), farPlane(farPlane)
{
    aspectRatio = static_cast<float>(width) / static_cast<float>(height);
    shadowProjectionMatrix = glm::perspective(glm::radians(90.0f), aspectRatio, nearPlane, farPlane);

    glGenFramebuffers(1, &depthCubemapFBO);

    // Create map
    glGenTextures(1, &depthCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);

    for (size_t i = 0; i < 6; i++)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
            width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    // Set up framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthCubemapFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

ShadowCubemap::~ShadowCubemap()
{}

void ShadowCubemap::Render(float* lightPos, std::vector<Model>& models, const int nModels, GUI& gui, CustomModel* customModel,
    ClothMesh* clothMesh)
{
    // Switch to proper viewport and bind framebuffer
    //glCullFace(GL_FRONT);
    glViewport(0, 0, width, height);
    glBindFramebuffer(GL_FRAMEBUFFER, depthCubemapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    std::vector<glm::mat4> lightSpaceMatrices(6);
    GetLightSpaceMatrices(lightPos, lightSpaceMatrices);

    shader.use();
    shader.setMat4Vector("lightSpaceMatrices", lightSpaceMatrices);
    shader.setVec3("lightPos", glm::vec3(lightPos[0], lightPos[1], lightPos[2]));
    shader.setFloat("farPlane", farPlane);

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

    //glCullFace(GL_BACK);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ShadowCubemap::GetLightSpaceMatrices(float* lightPos, std::vector<glm::mat4>& lightTransformMatrices)
{
    const glm::vec3 lightPosVec(lightPos[0], lightPos[1], lightPos[2]);

    lightTransformMatrices[0] = shadowProjectionMatrix * glm::lookAt(lightPosVec,
        lightPosVec + glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, -1.0f, 0.0f));

    lightTransformMatrices[1] = shadowProjectionMatrix * glm::lookAt(lightPosVec,
        lightPosVec + glm::vec3(-1.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, -1.0f, 0.0f));

    lightTransformMatrices[2] = shadowProjectionMatrix * glm::lookAt(lightPosVec,
        lightPosVec + glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 1.0f));

    lightTransformMatrices[3] = shadowProjectionMatrix * glm::lookAt(lightPosVec,
        lightPosVec + glm::vec3(0.0f, -1.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, -1.0f));

    lightTransformMatrices[4] = shadowProjectionMatrix * glm::lookAt(lightPosVec,
        lightPosVec + glm::vec3(0.0f, 0.0f, 1.0f),
        glm::vec3(0.0f, -1.0f, 0.0f));

    lightTransformMatrices[5] = shadowProjectionMatrix * glm::lookAt(lightPosVec,
        lightPosVec + glm::vec3(0.0f, 0.0f, -1.0f),
        glm::vec3(0.0f, -1.0f, 0.0f));
}
