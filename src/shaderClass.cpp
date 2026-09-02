#include "shader_class/shaderClass.h"

std::string getFileContents(const char* fileName) {
    std::ifstream in(fileName, std::ios::binary);
    if(in) {
        std::string contents;
        in.seekg(0, std::ios::end);
        contents.resize(in.tellg());
        in.seekg(0, std::ios::beg);
        in.read(&contents[0], contents.size());
        in.close();
        return contents;
    }
    throw(errno);
} 

Shaders::Shaders(const char* vertexFile, const char* fragmentFile) {
    std::string vertexCode = getFileContents(vertexFile);
    std::string fragmentCode = getFileContents(fragmentFile);

    std::cout << "Loading vertex shader: " << vertexFile << std::endl;
    std::cout << "Loading fragment shader: " << fragmentFile << std::endl;

    const char* vertexSource = vertexCode.c_str();
    const char* fragmentSource = fragmentCode.c_str();

    unsigned int vertexShader; 
    vertexShader = glCreateShader(GL_VERTEX_SHADER);

    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);

    int vertexSuccess;
    char vertexInfoLog[512];

    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &vertexSuccess);

    if(!vertexSuccess) {
        glGetShaderInfoLog(vertexShader, 512, NULL, vertexInfoLog);
        std::cout << "Error! vertex shader failed!" << vertexInfoLog << std::endl;
    }

    unsigned int fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);

    int fragmentSuccess;
    char fragmentInfoLog[512];

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &fragmentSuccess);

    if(!fragmentSuccess) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, fragmentInfoLog);
        std::cout << "Error! fragment shader failed!" << fragmentInfoLog << std::endl;
    }

    ID = glCreateProgram();

    glAttachShader(ID, vertexShader);
    glAttachShader(ID, fragmentShader);
    glLinkProgram(ID); 

    int shaderProgramSuccess;
    char shaderProgramInfoLog[512];

    glGetProgramiv(ID, GL_LINK_STATUS, &shaderProgramSuccess);
    if(!shaderProgramSuccess) {
        glGetProgramInfoLog(ID, 512, NULL, shaderProgramInfoLog);
        std::cout << "Error! linking shader to the program failed!" << shaderProgramInfoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void Shaders::Activate() {
    glUseProgram(ID);
}

void Shaders::Delete() {
    glDeleteProgram(ID);
}

void Shaders::setBool(std::string &uniformName, bool value) const {
    glUniform1i(glGetUniformLocation(ID, uniformName.c_str()), value);
}

void Shaders::setInt(std::string &uniformName, int value) const {
    glUniform1i(glGetUniformLocation(ID, uniformName.c_str()), value);
}

void Shaders::setFloat(std::string &uniformName, float value) const {
    glUniform1f(glGetUniformLocation(ID, uniformName.c_str()), value);
}

void Shaders::setFloat2(std::string &uniformName, float value1, float value2) const {
    glUniform2f(glGetUniformLocation(ID, uniformName.c_str()), value1, value2);
}
