#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <glm/gtc/type_ptr.hpp>

class Shader
{
public:
    unsigned int ID;
    // constructor generates the shader on the fly
    // ------------------------------------------------------------------------
    Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr)
    {
        // 1. retrieve the vertex/fragment source code from filePath
        std::string vertexCode;
        std::string fragmentCode;
        std::string geometryCode;
        std::ifstream vShaderFile;
        std::ifstream fShaderFile;
        std::ifstream gShaderFile;
        // ensure ifstream objects can throw exceptions:
        vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        gShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        try
        {
            // open files
            vShaderFile.open(vertexPath);
            fShaderFile.open(fragmentPath);
            std::stringstream vShaderStream, fShaderStream;
            // read file's buffer contents into streams
            vShaderStream << vShaderFile.rdbuf();
            fShaderStream << fShaderFile.rdbuf();
            // close file handlers
            vShaderFile.close();
            fShaderFile.close();
            // convert stream into string
            vertexCode = vShaderStream.str();
            fragmentCode = fShaderStream.str();
            // if geometry shader path is present, also load a geometry shader
            if (geometryPath != nullptr)
            {
                gShaderFile.open(geometryPath);
                std::stringstream gShaderStream;
                gShaderStream << gShaderFile.rdbuf();
                gShaderFile.close();
                geometryCode = gShaderStream.str();
            }
        }
        catch (std::ifstream::failure& e)
        {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
        }
        const char* vShaderCode = vertexCode.c_str();
        const char* fShaderCode = fragmentCode.c_str();
        // 2. compile shaders
        unsigned int vertex, fragment;
        // vertex shader
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, NULL);
        glCompileShader(vertex);
        checkCompileErrors(vertex, "VERTEX");
        // fragment Shader
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, NULL);
        glCompileShader(fragment);
        checkCompileErrors(fragment, "FRAGMENT");
        // if geometry shader is given, compile geometry shader
        unsigned int geometry;
        if (geometryPath != nullptr)
        {
            const char* gShaderCode = geometryCode.c_str();
            geometry = glCreateShader(GL_GEOMETRY_SHADER);
            glShaderSource(geometry, 1, &gShaderCode, NULL);
            glCompileShader(geometry);
            checkCompileErrors(geometry, "GEOMETRY");
        }
        // shader Program
        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        if (geometryPath != nullptr)
            glAttachShader(ID, geometry);
        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM");
        // delete the shaders as they're linked into our program now and no longer necessary
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        if (geometryPath != nullptr)
            glDeleteShader(geometry);

    }
    // activate the shader
    // ------------------------------------------------------------------------
    void use() const
    {
        glUseProgram(ID);
    }
    // utility uniform functions
    // ------------------------------------------------------------------------
    void setBool(const std::string& name, bool value) const
    {
        int uniform_location = glGetUniformLocation(ID, name.c_str());
        if (uniform_location == -1)
        {
            std::runtime_error("ERROR:SHADER::PROGRAM::UNIFORM:: Location with Name" + name + "Not Found or is Not in Use!");
            return;
        }

        glUniform1i(uniform_location, (int)value);
    }
    // ------------------------------------------------------------------------
    void setInt(const std::string& name, int value) const
    {
        int uniform_location = glGetUniformLocation(ID, name.c_str());
        if (uniform_location == -1)
        {
            std::runtime_error("ERROR:SHADER::PROGRAM::UNIFORM:: Location with Name" + name + "Not Found or is Not in Use!");
            return;
        }

        glUniform1i(uniform_location, value);
    }
    // ------------------------------------------------------------------------
    void setFloat(const std::string& name, float value) const
    {
        int uniform_location = glGetUniformLocation(ID, name.c_str());
        if (uniform_location == -1)
        {
            std::runtime_error("ERROR:SHADER::PROGRAM::UNIFORM:: Location with Name" + name + "Not Found or is Not in Use!");
            return;
        }

        glUniform1f(uniform_location, value);
    }
    // ------------------------------------------------------------------------
    void setVec2(const std::string& name, const glm::vec2& value) const
    {
        int uniform_location = glGetUniformLocation(ID, name.c_str());
        if (uniform_location == -1)
        {
            std::runtime_error("ERROR:SHADER::PROGRAM::UNIFORM:: Location with Name" + name + "Not Found or is Not in Use!");
            return;
        }

        glUniform2fv(uniform_location, 1, &value[0]);
    }
    void setVec2(const std::string& name, float x, float y) const
    {
        int uniform_location = glGetUniformLocation(ID, name.c_str());
        if (uniform_location == -1)
        {
            std::runtime_error("ERROR:SHADER::PROGRAM::UNIFORM:: Location with Name" + name + "Not Found or is Not in Use!");
            return;
        }

        glUniform2f(uniform_location, x, y);
    }
    // ------------------------------------------------------------------------
    void setVec3(const std::string& name, const glm::vec3& value) const
    {
        int uniform_location = glGetUniformLocation(ID, name.c_str());
        if (uniform_location == -1)
        {
            std::runtime_error("ERROR:SHADER::PROGRAM::UNIFORM:: Location with Name" + name + "Not Found or is Not in Use!");
            return;
        }

        glUniform3fv(uniform_location, 1, &value[0]);
    }
    void setVec3(const std::string& name, float x, float y, float z) const
    {
        int uniform_location = glGetUniformLocation(ID, name.c_str());
        if (uniform_location == -1)
        {
            std::runtime_error("ERROR:SHADER::PROGRAM::UNIFORM:: Location with Name" + name + "Not Found or is Not in Use!");
            return;
        }

        glUniform3f(uniform_location, x, y, z);
    }
    // ------------------------------------------------------------------------
    void setVec4(const std::string& name, const glm::vec4& value) const
    {
        int uniform_location = glGetUniformLocation(ID, name.c_str());
        if (uniform_location == -1)
        {
            std::runtime_error("ERROR:SHADER::PROGRAM::UNIFORM:: Location with Name" + name + "Not Found or is Not in Use!");
            return;
        }

        glUniform4fv(uniform_location, 1, &value[0]);
    }
    void setVec4(const std::string& name, float x, float y, float z, float w) const
    {
        int uniform_location = glGetUniformLocation(ID, name.c_str());
        if (uniform_location == -1)
        {
            std::runtime_error("ERROR:SHADER::PROGRAM::UNIFORM:: Location with Name" + name + "Not Found or is Not in Use!");
            return;
        }

        glUniform4f(uniform_location, x, y, z, w);
    }
    // ------------------------------------------------------------------------
    void setMat2(const std::string& name, const glm::mat2& mat) const
    {
        int uniform_location = glGetUniformLocation(ID, name.c_str());
        if (uniform_location == -1)
        {
            std::runtime_error("ERROR:SHADER::PROGRAM::UNIFORM:: Location with Name" + name + "Not Found or is Not in Use!");
            return;
        }

        glUniformMatrix2fv(uniform_location, 1, GL_FALSE, &mat[0][0]);
    }
    // ------------------------------------------------------------------------
    void setMat3(const std::string& name, const glm::mat3& mat) const
    {
        int uniform_location = glGetUniformLocation(ID, name.c_str());
        if (uniform_location == -1)
        {
            std::runtime_error("ERROR:SHADER::PROGRAM::UNIFORM:: Location with Name" + name + "Not Found or is Not in Use!");
            return;
        }

        glUniformMatrix3fv(uniform_location, 1, GL_FALSE, &mat[0][0]);
    }
    // ------------------------------------------------------------------------
    void setMat4(const std::string& name, const glm::mat4& mat) const
    {
        int uniform_location = glGetUniformLocation(ID, name.c_str());
        if (uniform_location == -1)
        {
            std::runtime_error("ERROR:SHADER::PROGRAM::UNIFORM:: Location with Name" + name + "Not Found or is Not in Use!");
            return;
        }

        glUniformMatrix4fv(uniform_location, 1, GL_FALSE, &mat[0][0]);
    }

    void setMat4Vector(const std::string& name, const std::vector<glm::mat4>& mat_vec) const
    {
        int uniform_location = glGetUniformLocation(ID, name.c_str());
        if (uniform_location == -1)
        {
            std::runtime_error("ERROR:SHADER::PROGRAM::UNIFORM:: Location with Name" + name + "Not Found or is Not in Use!");
            return;
        }

        glUniformMatrix4fv(uniform_location, (GLsizei)mat_vec.size(), GL_FALSE, glm::value_ptr(mat_vec[0]));
    }

private:
    // utility function for checking shader compilation/linking errors.
    // ------------------------------------------------------------------------
    void checkCompileErrors(GLuint shader, std::string type)
    {
        GLint success;
        GLchar infoLog[1024];
        if (type != "PROGRAM")
        {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
        else
        {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success)
            {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
    }
};