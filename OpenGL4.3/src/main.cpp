// Trying my first compute shader with a bunch of particles bouncing in a box

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include "../header/shader_class/shaderClass.h"

static const unsigned int width = 900;
static const unsigned int height = 600;

void framebufferSizeCallback(GLFWwindow * window, int Width, int Height) {
    glViewport(0, 0, Width, Height);
}

int main(void) {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); 
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(width, height, "Smoothed particles hydrodynamics simulation", NULL, NULL);

    if(window == NULL) {
        std::cerr << "Failed to create a window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);

    int version = gladLoadGL(glfwGetProcAddress);
    if (version == 0) {
        std::cerr << "Failed to initialize OpenGL context" << std::endl;
        return -1;
    }

    glViewport(0, 0, width, height);

    const unsigned int particlesNumber = 1200;

    Shaders shaderProgram = Shaders("shader/shader.vert", "shader/shader.frag");
    shaderProgram.Activate();
    shaderProgram.AddComputeShader("shader/shader.comp");
    shaderProgram.ActivateComputeShader({1200, 1, 1}, GL_ALL_BARRIER_BITS);

    while(!glfwWindowShouldClose(window)) {
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    shaderProgram.Delete();
    shaderProgram.DeleteComputeShader();
    glfwTerminate();
    return 0;
}
