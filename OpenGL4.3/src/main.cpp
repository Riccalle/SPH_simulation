// Trying my first compute shader with a bunch of particles bouncing in a box

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "../header/shader_class/shaderClass.h"

static const unsigned int width = 900;
static const unsigned int height = 600;

void framebufferSizeCallback(GLFWwindow * window, int Width, int Height) {
    glViewport(0, 0, Width, Height);
}

struct Particle {
    glm::vec4 pos;
    glm::vec4 vel;
};

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

    srand(glfwGetTime());
    const unsigned int particlesNumber = 38400;
    const float radius = 0.08f;
    std::array<Particle, particlesNumber> particles;
    for (int i = 0; i < particlesNumber; i++) {
        particles[i].pos.x = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
        particles[i].pos.y = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
        particles[i].pos.z = 0.0f;
        particles[i].pos.w = 0.0f;

        particles[i].vel.x = 0.5f * (((float)rand() / RAND_MAX) * 2.0f - 1.0f);
        particles[i].vel.y = 0.5f * (((float)rand() / RAND_MAX) * 2.0f - 1.0f);
        particles[i].vel.z = 0.0f;
        particles[i].vel.w = 0.0f;
    }

    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    GLuint SSBO;
    glGenBuffers(1, &SSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Particle) * particlesNumber, particles.data(), GL_DYNAMIC_COPY);
    // This function specifies how much memory we want to allocate on the GPU
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, SSBO); 
    // This function makes it so the data contained in the SSBO in visibile under binding = 0 to other shaders

    // Shaders

    Shaders shaderProgram = Shaders("shader/shader.vert", "shader/shader.frag");
    shaderProgram.Activate();
    shaderProgram.AddComputeShader("shader/shader.comp");
    shaderProgram.ActivateComputeShader({particlesNumber / 120, 1, 1}, GL_ALL_BARRIER_BITS);

    glEnable(GL_PROGRAM_POINT_SIZE);

    glUniform1f(glGetUniformLocation(shaderProgram.computeID, "radius"), radius);
    int deltaTimeUniformLocation = glGetUniformLocation(shaderProgram.computeID, "deltaTime");

    double lastTime = glfwGetTime();
    double thisTime;
    double deltaTime;
    double timer = 0;

    while(!glfwWindowShouldClose(window)) {
        thisTime = glfwGetTime();
        deltaTime = thisTime - lastTime;
        lastTime = thisTime;

        if (thisTime - timer > 1) {
            std::cout << "\r\033[32mFrames per second: " << 1 / deltaTime << std::flush;
            timer = thisTime;
        }

        shaderProgram.ActivateComputeShader({particlesNumber / 128, 1, 1}, GL_ALL_BARRIER_BITS);
        glUniform1f(deltaTimeUniformLocation, deltaTime);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        shaderProgram.Activate();
        glBindVertexArray(VAO);
        glDrawArrays(GL_POINTS, 0, particlesNumber);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    std::cout << "\n";
    shaderProgram.Delete();
    shaderProgram.DeleteComputeShader();
    glfwTerminate();
    return 0;
}
