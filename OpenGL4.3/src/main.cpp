#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <iostream>

int main(void) {
    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;
    return 0;
}
