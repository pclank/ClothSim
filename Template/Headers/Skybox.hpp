#pragma once

#include <glad/glad.h>
#include <iostream>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <stb_image.h>
#include <Shader.hpp>

// Used as postfixes for the image files
const std::vector<std::string> postfixes = {
    "right.jpg",    // +X
    "left.jpg",     // -X
    "top.jpg",      // +Y
    "bottom.jpg",   // -Y
    "front.jpg",    // +Z
    "back.jpg"      // -Z
};

// The skybox vertices
const float box_vertices[] = {
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};

/// <summary>
/// Implements Skyboxes using Cubemaps
/// </summary>
class Skybox
{
public:
    // Constructor
    Skybox(std::string file_name, Shader shader);

    /// <summary>
    /// Returns cubemap ID
    /// </summary>
    /// <returns></returns>
    unsigned int GetCubemapID();

    /// <summary>
    /// Render Skybox
    /// </summary>
    void Render(glm::mat4 viewMat, glm::mat4 projMat);

private:
    unsigned int VBO, VAO;          // VBO and VAO of Skybox
    Shader shader;                  // Shader of Skybox
    std::string file_name;			// Prefix of name of Texture files
    unsigned int cubemap_id = -1;	// The Texture ID of the Cubemap
};