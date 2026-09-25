#ifndef SHADER_CLASS_H
#define SHADER_CLASS_H

#include <glad/glad.h>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <cerrno>

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

#endif
