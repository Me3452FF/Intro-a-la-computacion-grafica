#include "shaderClass.h"

#include <fstream>
#include <sstream>
#include <iostream>

Shader::Shader(
    const char* vertexFile,
    const char* fragmentFile
)
{
    std::string vertexCode;
    std::string fragmentCode;

    std::ifstream vertexFileStream(vertexFile);
    std::ifstream fragmentFileStream(fragmentFile);

    if (!vertexFileStream.is_open())
    {
        std::cout
            << "ERROR: No se pudo abrir "
            << vertexFile
            << std::endl;
    }

    if (!fragmentFileStream.is_open())
    {
        std::cout
            << "ERROR: No se pudo abrir "
            << fragmentFile
            << std::endl;
    }

    std::stringstream vertexStream;
    std::stringstream fragmentStream;

    vertexStream << vertexFileStream.rdbuf();
    fragmentStream << fragmentFileStream.rdbuf();

    vertexFileStream.close();
    fragmentFileStream.close();

    vertexCode = vertexStream.str();
    fragmentCode = fragmentStream.str();

    const char* vertexSource =
        vertexCode.c_str();

    const char* fragmentSource =
        fragmentCode.c_str();

    // VERTEX SHADER

    GLuint vertexShader =
        glCreateShader(
            GL_VERTEX_SHADER
        );

    glShaderSource(
        vertexShader,
        1,
        &vertexSource,
        NULL
    );

    glCompileShader(vertexShader);

    // FRAGMENT SHADER

    GLuint fragmentShader =
        glCreateShader(
            GL_FRAGMENT_SHADER
        );

    glShaderSource(
        fragmentShader,
        1,
        &fragmentSource,
        NULL
    );

    glCompileShader(fragmentShader);

    // SHADER PROGRAM

    ID = glCreateProgram();

    glAttachShader(
        ID,
        vertexShader
    );

    glAttachShader(
        ID,
        fragmentShader
    );

    glLinkProgram(ID);

    // BORRAR SHADERS INDIVIDUALES

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void Shader::Activate()
{
    glUseProgram(ID);
}

void Shader::Delete()
{
    glDeleteProgram(ID);
}