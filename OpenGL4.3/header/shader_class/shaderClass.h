#ifndef SHADER_CLASS_H
#define SHADER_CLASS_H

#include <glad/gl.h>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <cerrno>
#include <array>

std::string getFileContents(const char* fileName);

class Shaders 
{
    public:
        GLuint ID;
        Shaders(const char* vertexFile, const char* fragmentFile);

        void Activate();
        void Delete();

        void setBool(std::string &uniformName, bool value) const;
        void setInt(std::string &uniformName, int value) const;
        void setFloat(std::string &uniformName, float value) const;
        void setFloat2(std::string &uniformName, float value1, float value2) const;
};

class ComputeShader {
    public:
        ComputeShader(const char * directory);
        GLuint ID;

        void Activate(std::array<int, 3> workGroup, GLenum memoryBarrier);
        void Delete();
};

#endif
