#include <Skybox.hpp>

Skybox::Skybox(const std::string file_name, const Shader shader) : file_name(file_name), shader(shader)
{
    // Generate and bind cubemap ID
    unsigned int cubemapID;
    glGenTextures(1, &cubemapID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapID);

    // Load all 6 textures
    int img_width, img_height, channels_n;
    for (unsigned int i = 0; i < 6; i++)
    {
        // Load data
        const std::string img_file = file_name + postfixes[i];
        unsigned char* img_data = stbi_load(img_file.c_str(), &img_width, &img_height, &channels_n, 0);
        if (img_data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, GL_RGB, img_width, img_height, 0, GL_RGB, GL_UNSIGNED_BYTE, img_data
            );
            stbi_image_free(img_data);
        }
        // Catch loading errors
        else
        {
            std::cout << "Cubemap tex failed to load at path: " << img_file << std::endl;
            stbi_image_free(img_data);
        }
    }

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    cubemap_id = cubemapID;

    // Generate VAO and VBO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(box_vertices), &box_vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindVertexArray(0);
}

void Skybox::Render(glm::mat4 viewMat, glm::mat4 projMat)
{
    // "Disable" Depth Mask
    glDepthFunc(GL_LEQUAL);

    // Remove translation from view matrix
    viewMat = glm::mat4(glm::mat3(viewMat));

    shader.use();
    shader.setMat4("viewMatrix", viewMat);
    shader.setMat4("projectionMatrix", projMat);

    glBindVertexArray(VAO);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap_id);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // "Re-enable" Depth Mask
    glDepthFunc(GL_LESS);
}

unsigned int Skybox::GetCubemapID()
{
    return cubemap_id;
}