#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

static const unsigned int width = 900;
static const unsigned int height = 600;

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

    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to load GLAD" << std::endl;
        return -1;
    }

    glViewport(0, 0, width, height);

    while(!glfwWindowShouldClose(window)) {
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    std::cout << "\n" << glGetString(GL_VERSION) << std::endl;
    glfwTerminate();

    return 0;
}
