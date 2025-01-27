// Preprocessor Directives
#ifndef GLITTER
#define GLITTER
#pragma once

// System Headers
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <btBulletDynamicsCommon.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <CL/cl.hpp>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

// Define Some Constants
const int mWidth = 1920;
const int mHeight = 1080;

const size_t MAX_MODELS = 10;

// **********************************************************************************
// OpenCL section
// **********************************************************************************

cl::Device default_device;
cl::Context context;
std::string kernel_source;
cl::Program::Sources sources;
cl::CommandQueue queue;
cl::Program program;
cl::Buffer test_buffer;
cl::Buffer debug_buffer;
cl::Kernel test_kernel;
cl::NDRange global_tex(mWidth, mHeight);

float hardcoded_vertices[] = {
    // positions          // colors           // texture coords
     0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right
     0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
    -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
    -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // top left 
};

float quadVertices[] = {
    // positions        // texture Coords
    -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
    -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
     1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
     1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
};

//std::vector<float> customDebug = {
//    // positions
//    -0.5f, -0.5f, 0.0f,
//    0.5f, -0.5f, 0.0f,
//    0.5f, 0.5f, 0.0f,
//    -0.5f, 0.5f, 0.0f
//};
std::vector<float> customDebug = {
    // positions
    0.5f,  0.5f, 0.0f,  // top right
     0.5f, -0.5f, 0.0f,  // bottom right
    -0.5f, -0.5f, 0.0f,  // bottom left
    -0.5f,  0.5f, 0.0f   // top left 
};

unsigned int indices[] = {
    0, 1, 3, // first triangle
    1, 2, 3  // second triangle
};

unsigned int quadVAO = 0;
unsigned int quadVBO;
inline void renderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}


cl::make_kernel<cl::Image2D> tester(test_kernel);

cl::Image2D target_texture;

std::string ReadFile2(const char* f_name = "kernels.cl")
{
    std::string kernel_code;
    std::ifstream kernel_file;
    // ensure ifstream objects can throw exceptions:
    kernel_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        
        kernel_file.open(f_name);
        std::stringstream kernel_stream;
        
        kernel_stream << kernel_file.rdbuf();
        
        kernel_file.close();
        
        kernel_code = kernel_stream.str();
    }
    catch (std::ifstream::failure& e)
    {
        std::cout << "ERROR::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
    }

    // return char sequence
    return kernel_code.c_str();
}

#endif //~ Glitter Header
