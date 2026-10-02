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

    // VAO
    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Data

    unsigned int division = 20;
    std::vector<GLfloat> meshVert = {0.0f, 0.0f, 0.0f};
    std::vector<GLuint> meshIndices;
    for (int i = 0; i < division; i++) {
        float angle = i * 2 * (float)M_PI / division;

        meshVert.push_back(cosf(angle));
        meshVert.push_back(sinf(angle));
        meshVert.push_back(0.0f);

        int X = 0;
        int Y = i + 1;
        int Z = (i + 1) % division + 1;

        meshIndices.push_back(X);
        meshIndices.push_back(Y);
        meshIndices.push_back(Z);
    }

    meshVert.shrink_to_fit();
    meshIndices.shrink_to_fit();

    // meshVBO
    GLuint meshVBO;
    glGenBuffers(1, &meshVBO);

    glBindBuffer(GL_ARRAY_BUFFER, meshVBO);
    glBufferData(GL_ARRAY_BUFFER, meshVert.size() * sizeof(GLfloat), meshVert.data(), GL_STATIC_DRAW);

    // meshEBO
    GLuint meshEBO;
    glGenBuffers(1, &meshEBO);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, meshIndices.size() * sizeof(GLuint), meshIndices.data(), GL_STATIC_DRAW);

    GLuint SSBO;
    glGenBuffers(1, &SSBO);
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Particle) * particlesNumber, particles.data(), GL_DYNAMIC_COPY);
    // This function specifies how much memory we want to allocate on the GPU
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, SSBO); 
    // This function makes it so the data contained in the SSBO in visibile under binding = 0 to other shaders

    // Grid for neighbor research
    float cellSize = radius * 2.0f;
    unsigned int gridResX = (unsigned int)std::ceil(2.0f / cellSize);
    unsigned int gridResY = (unsigned int)std::ceil(2.0f / cellSize);
    unsigned int totCell = gridResX * gridResY;

    int cellHead[totCell] = {-1}; // flag -1 if the cell is empty
    int particleNext[particlesNumber] = {-1}; // flag -1 if it contains the last particle

    // New SSBOs for neighbor research
    GLuint cellHeadSSBO; 
    glGenBuffers(1, &cellHeadSSBO);
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, cellHeadSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(cellHead), &cellHead, GL_DYNAMIC_COPY);
    
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, cellHeadSSBO);

    GLuint particleNextSSBO;
    glGenBuffers(1, &particleNextSSBO);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleNextSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(particleNext), &particleNext, GL_DYNAMIC_COPY);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, particleNextSSBO);

    // Attributes

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    // Shaders

    Shaders shaderProgram = Shaders("shader/shader.vert", "shader/shader.frag");
    shaderProgram.Activate();
    ComputeShader computeShaderProgram = ComputeShader("shader/shader.comp");
    
    glUniform1f(glGetUniformLocation(shaderProgram.ID, "radius"), radius);
    computeShaderProgram.Activate({particlesNumber / 128, 1, 1}, GL_ALL_BARRIER_BITS);
    glUniform1f(glGetUniformLocation(computeShaderProgram.ID, "radius"), radius);
    int deltaTimeUniformLocation = glGetUniformLocation(computeShaderProgram.ID, "deltaTime");

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

        computeShaderProgram.Activate({particlesNumber / 128, 1, 1}, GL_ALL_BARRIER_BITS);
        glUniform1f(deltaTimeUniformLocation, deltaTime);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        shaderProgram.Activate();
        glBindVertexArray(VAO);
        glDrawElementsInstanced(GL_TRIANGLES, meshIndices.size(), GL_UNSIGNED_INT, 0, particlesNumber);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    std::cout << "\n";

    shaderProgram.Delete();
    computeShaderProgram.Delete();

    glfwTerminate();
    return 0;
}
