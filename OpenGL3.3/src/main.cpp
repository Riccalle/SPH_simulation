// Doing my first SPH simulation 
// Weakly Compressible Smoothed Particles Hydrodynamics
// TODO: change hashing method in neighbor research to not kill performance
//       add GUI
//       maybe optimize more
//       add color

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>
#include <array>
#include <string>
#include <cmath>
#include <unordered_map>

#define PI 3.1415926897932f

#include <shader_class/shaderClass.h>
#include <SPHheader/NeighborResearch.h>
#include <SPHheader/SmoothingKernels.h>
#include <SPHheader/spawner2D.h>

static const unsigned int width = 1200;
static const unsigned int height = 900;
std::array<float, 2> windowRatio = {(float)height / (float)width, (float)width / (float)height};

struct ExtForce {
    glm::vec3 pos = glm::vec3(0.0f);
    float magnitude = 71.0f;
    float force = 0.0f;
    float influenceRadius = 0.12f;
};

ExtForce extForce;

void processInput(GLFWwindow* window, double deltaTime) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) 
        glfwSetWindowShouldClose(window, true);
    
    double mouseXpos;
    double mouseYpos;
    glfwGetCursorPos(window, &mouseXpos, &mouseYpos);
    float normX = ((float)mouseXpos / (float)width) * 2.0f - 1.0f;
    float normY = -((float)mouseYpos / (float)height) * 2.0f + 1.0f;
    extForce.pos = glm::vec3(normX, normY, 0.0f);

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) 
        extForce.force = extForce.magnitude;
    else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) 
        extForce.force = -extForce.magnitude;
    else 
        extForce.force = 0.0f;
}

void framebufferSizeCallback(GLFWwindow * window, int Width, int Height) {
    glViewport(0, 0, Width, Height);
}

#pragma region variables

/*
    In base al numero di particelle che si vuole,
    cambiare il size, seguendo circa s = sqrt(A / N) dove A
    è l'area che si vuole riempire.
    Cambiare il raggio d'influenza, usando più o meno 2.2 * s.
    Cambiare la massa, con targetDensity * s * s.
    Raggio visivo a piacimento. Consigliato 0.5 * s.
    Se si aumentano le particelle, diminuire il time step.
    Se si riduce molto il raggio d'influenza, mettere artificialDeltaTime 
    circa 0.4f * Hradius * sqrt(stiffness).
    Il jitterstrenght circa 0.05f * s.
*/

const float radius =         0.007f;
const float mass =            1.11f;
const float gravity =         15.8f;
const float damping =          0.3f;
const float Hradius =        0.029f;
const float targetDensity = 1800.0f;
const float spawnDensity =  4700.0f;
const float jitterStrenght = 0.002f;
const float viscosity =        6.0f;
const float stiffness =       70.0f;

const float pressGradKerConst = -30.0f / (M_PI * pow(Hradius, 5));
const float denKerConst = 4.0f / (M_PI * pow(Hradius, 8));
const float viscKerConst = 40.0f / (M_PI * pow(Hradius, 5));

glm::vec3 color = glm::vec3(0.0f, 0.6f, 1.0f);

unsigned int partNum;
const int division = 10;

#pragma endregion

void computeAcceleration(std::vector<glm::vec3> &acc, std::vector<glm::vec3> &vel, std::vector<glm::vec3> &pos) {
    unsigned int size = acc.size();

    static std::vector<glm::vec3> totForce;
    static std::vector<float> density;
    static std::vector<float> pressure;

    if (totForce.size() != size) {
        totForce.resize(size);
        density.resize(size);
        pressure.resize(size);
    }

    std::fill(totForce.begin(), totForce.end(), glm::vec3(0.0f));
    std::fill(density.begin(), density.end(), 0.0f);
    std::fill(pressure.begin(), pressure.end(), 0.0f);

    for(int i = 0; i < size; i++) {
        for(int j : NeighborResearch::neighbors[i]) {
            float dx = pos[i].x - pos[j].x;
            float dy = pos[i].y - pos[j].y;
            float dr = sqrt(dx * dx + dy * dy);

            density[i] += mass * densityKernel(dr, Hradius, denKerConst);
        }
    }

    computePressure(targetDensity, stiffness, density, pressure);

    for (int i = 0; i < size; i++) {
        glm::vec3 delta = extForce.pos - pos[i];

        if(glm::length(delta) > extForce.influenceRadius)
            continue;

        if (extForce.force != 0.0f && glm::length(delta) > 1e-4f){
            totForce[i] += glm::normalize(delta) * extForce.force * 6000.0f; // scaling it so it does something
        }
    }

    for(int i = 0; i < size; i++) {
        glm::vec3 tempForce = glm::vec3(0.0f);

        for(int j : NeighborResearch::neighbors[i]) {
            tempForce += pressureForce(i, j, pos, pressure, density, mass, Hradius, pressGradKerConst);
            tempForce += viscosityForce(i, j, pos, vel, density, mass, Hradius, viscosity, viscKerConst);
        }

        totForce[i] += tempForce;
        if(density[i] > 0) 
            acc[i] = totForce[i] / density[i] + glm::vec3(0.0f, -gravity, 0.0f);
        else 
            acc[i] = glm::vec3(0.0f, -gravity, 0.0f);
    }
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
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

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to load GLAD" << std::endl;
        return -1;
    }

    glViewport(0, 0, width, height);

    // VAO 

    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Data

    std::vector<float> meshVert = {
        0.0f, 0.0f, 0.0f, color.x, color.y, color.z
    };

    std::vector<GLuint> meshIndices = {};

    for(int i = 0; i < division; i++) {
        float angle = i * 2 * PI / division;

        float x = radius * cosf(angle);
        float y = radius * sinf(angle);

        meshVert.push_back(x);
        meshVert.push_back(y);
        meshVert.push_back(0.0f);

        meshVert.push_back(color.x);
        meshVert.push_back(color.y);
        meshVert.push_back(color.z);

        int X = 0;
        int Y = i + 1;
        int Z = i + 2 <= division ? i + 2 : 1;

        meshIndices.push_back(X);
        meshIndices.push_back(Y);
        meshIndices.push_back(Z);
    }

    srand(glfwGetTime());

    std::vector<glm::vec3> pos = {};
    std::vector<glm::vec3> vel = {};
    std::vector<glm::vec3> acc = {};

    // Initializing 

    SpawnRegion spawnRegion = {
        glm::vec2(-0.25f, -0.2f), 
        glm::vec2(0.6f, 0.6f)
    };

    pos = pointGrid(spawnRegion, spawnDensity, jitterStrenght);

    for (int i = 0; i < pos.size(); i++) {
        vel.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
        acc.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
    }

    // meshVBO

    GLuint meshVBO;
    glGenBuffers(1, &meshVBO);

    glBindBuffer(GL_ARRAY_BUFFER, meshVBO);
    glBufferData(GL_ARRAY_BUFFER, meshVert.size() * sizeof(float), meshVert.data(), GL_STATIC_DRAW);

    // dynamicVBO 

    GLuint dynVBO;
    glGenBuffers(1, &dynVBO);

    glBindBuffer(GL_ARRAY_BUFFER, dynVBO);
    glBufferData(GL_ARRAY_BUFFER, pos.size() * sizeof(glm::vec3), pos.data(), GL_DYNAMIC_DRAW);

    // Velocity VBO 
    GLuint velVBO;
    glGenBuffers(1, &velVBO);

    glBindBuffer(GL_ARRAY_BUFFER, velVBO);
    glBufferData(GL_ARRAY_BUFFER, vel.size() * sizeof(glm::vec3), vel.data(), GL_DYNAMIC_DRAW);

    // EBO 

    GLuint EBO;
    glGenBuffers(1, &EBO);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, meshIndices.size() * sizeof(unsigned int), meshIndices.data(), GL_STATIC_DRAW);

    // Attributes for meshVBO

    glBindBuffer(GL_ARRAY_BUFFER, meshVBO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Attributes for dynVBO

    glBindBuffer(GL_ARRAY_BUFFER, dynVBO);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(2);

    glVertexAttribDivisor(2, 1);

    // Attributes for velVBO

    glBindBuffer(GL_ARRAY_BUFFER, velVBO);

    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(3);

    glVertexAttribDivisor(3, 1);

    // Shaders

    Shaders shaderProgram = Shaders("shaders/shader.vert", "shaders/shader.frag");

    shaderProgram.Activate();

    // Main code that runs once

    std::cout << "Number of particles: " << pos.size() << std::endl;
    NeighborResearch NResearch = NeighborResearch(radius, Hradius);

    double time;
    double lastTime = glfwGetTime();
    double timer = glfwGetTime();

    double simulationTime;
    double startTime = glfwGetTime();

    double deltaTime;
    const float artificialDeltaTime = 1.0f / 1000.0f;
    const float horizontalBoundBox = 0.7f;
    const float verticalBoxBound = 1.0f;

    while(!glfwWindowShouldClose(window)) {
        time = glfwGetTime();
        deltaTime = time - lastTime;

        if(time - timer > 1) {
            std::cout << "\r\033[32mFrames per second: " << 1 / deltaTime << std::flush;
            timer = time;
        }

        lastTime = time;
        simulationTime = glfwGetTime() - startTime;

        processInput(window, deltaTime); 

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        NResearch.updateResearch(pos);
        computeAcceleration(acc, vel, pos);

        for(int i = 0; i < pos.size(); i++) {

            vel[i] += acc[i] * artificialDeltaTime;
            pos[i] += vel[i] * artificialDeltaTime;

            if(pos[i].x + radius > horizontalBoundBox) {
                pos[i].x = horizontalBoundBox - radius;
                vel[i].x *= -damping;
            }

            if(pos[i].x - radius < -horizontalBoundBox) {
                pos[i].x = radius - horizontalBoundBox;
                vel[i].x *= -damping;
            }

            if(pos[i].y + radius >  verticalBoxBound) {
                pos[i].y = verticalBoxBound - radius;
                vel[i].y *= -damping;
            }

            if(pos[i].y - radius < -verticalBoxBound) {
                pos[i].y = radius - verticalBoxBound;
                vel[i].y *= -damping;
            }

            if (!std::isfinite(pos[i].x) || !std::isfinite(pos[i].y)) {
                pos[i] = glm::vec3(0.0f);
                vel[i] = glm::vec3(0.0f);
            } // Quickly checking whether any of my particles are not a number or infinity. 
        }

        glBindBuffer(GL_ARRAY_BUFFER, dynVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, pos.size() * sizeof(glm::vec3), pos.data());

        glBindBuffer(GL_ARRAY_BUFFER, velVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, vel.size() * sizeof(glm::vec3), vel.data());

        glBindVertexArray(VAO);
        glDrawElementsInstanced(GL_TRIANGLES, meshIndices.size(), GL_UNSIGNED_INT, (void*)0, pos.size());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    shaderProgram.Delete();
    glfwTerminate();
    return 0;
}
