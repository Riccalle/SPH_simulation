#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "../header/shader_class/shaderClass.h"
#include "../header/SPHheader/spawner2D.h"

static const unsigned int width = 900;
static const unsigned int height = 600;

void framebufferSizeCallback(GLFWwindow * window, int Width, int Height) {
    glViewport(0, 0, Width, Height);
}

struct Particle {
    glm::vec4 pos; 
    glm::vec4 vel;
    /* Hardawre rules states that allocating vector memory takes up
       16 bytes of memory, so we use vec4 instead of vec3
       Since we don't use the z and w component of neither
       vel or pos, instead of creating a density variable,
       We can just use pos w component
    */ 
};

glm::vec3 mouseInfo;

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) 
        glfwSetWindowShouldClose(window, true);
    
    int currentWidth, currentHeight;
    glfwGetWindowSize(window, &currentWidth, &currentHeight);

    double mouseXpos;
    double mouseYpos;
    glfwGetCursorPos(window, &mouseXpos, &mouseYpos);
    if (currentWidth > 0 && currentHeight > 0) {
        float normX = ((float)mouseXpos / (float)currentWidth) * 2.0f - 1.0f;
        float normY = -((float)mouseYpos / (float)currentHeight) * 2.0f + 1.0f;
        mouseInfo.x = normX;
        mouseInfo.y = normY;
    }

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) 
        mouseInfo.z = -1.0f;
    else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) 
        mouseInfo.z = 1.0f;
    else 
        mouseInfo.z = 0.0f;
}

void resetParticles(SpawnRegion spawnRegion, int particlesNumber, std::vector<Particle> &particles) {
    float spawnDensity = particlesNumber / (spawnRegion.size.x * spawnRegion.size.y);
    std::vector<glm::vec3> pos = std::vector<glm::vec3>();
    particles = std::vector<Particle>(particlesNumber);
    float jitterStrenght = 0.0f;
    if (particlesNumber < 12000) {
        glm::ivec2 spawnCountPerAxis = calculateSpawnCountPerAxis(spawnRegion.size, spawnDensity);
        float tx = spawnRegion.size.x / (spawnCountPerAxis.x - 1);
        jitterStrenght = tx / 20.0f; // Jitter strenght is 5% of the horizontal difference bitween the particles
        if (jitterStrenght < 1e-4f) jitterStrenght = 0.0f; 
    }
    pos = pointGrid(spawnRegion, spawnDensity, 0.0f);

    for (int i = 0; i < particlesNumber; i++) {
        particles[i].pos.x = pos[i].x;
        particles[i].pos.y = pos[i].y;
        particles[i].pos.z = 0.0f;
        particles[i].pos.w = 0.0f;

        particles[i].vel.x = 0.0f;
        particles[i].vel.y = 0.0f;
        particles[i].vel.z = 0.0f;
        particles[i].vel.w = 0.0f;
    }
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
    glfwSwapInterval(1);

    int version = gladLoadGL(glfwGetProcAddress);
    if (version == 0) {
        std::cerr << "Failed to initialize OpenGL context" << std::endl;
        return -1;
    }

    glViewport(0, 0, width, height);

    srand(glfwGetTime());
    int particlesNumber = 38400;
    float radius = 0.005f;

    SpawnRegion spawnRegion = {
        glm::vec2(0.0f, 0.0f),
        glm::vec2(0.7f, 0.7f)
    };

    std::vector<Particle> particles(particlesNumber);
    resetParticles(spawnRegion, particlesNumber, particles);

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

    // New SSBOs for neighbor research
    GLuint cellHeadSSBO; 
    glGenBuffers(1, &cellHeadSSBO);
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, cellHeadSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, totCell * sizeof(int), nullptr, GL_DYNAMIC_COPY);
    
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, cellHeadSSBO);

    GLuint particleNextSSBO;
    glGenBuffers(1, &particleNextSSBO);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleNextSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, particlesNumber * sizeof(int), nullptr, GL_DYNAMIC_COPY);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, particleNextSSBO);

    // Attributes

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    // Shaders

    Shaders shaderProgram = Shaders("shader/shader.vert", "shader/shader.frag");
    shaderProgram.Activate();
    ComputeShader computeShaderProgram = ComputeShader("shader/shader.comp");
    
    glUniform1f(glGetUniformLocation(shaderProgram.ID, "radius"), radius);
    glUseProgram(computeShaderProgram.ID);
    GLint radiusUniLoc = glGetUniformLocation(computeShaderProgram.ID, "radius");
    glUniform1f(radiusUniLoc, radius);
    glUniform2i(glGetUniformLocation(computeShaderProgram.ID, "gridRes"), gridResX, gridResY);
    int phaseUniformLocation = glGetUniformLocation(computeShaderProgram.ID, "phase");

    float mass = 0.013f;
    float stiffness = 200.0f;
    float targetDensity = 400.0f;
    float collisionDamping = 0.4f;
    float gravity = 9.8f;
    float fixedDeltaTime = 0.0008f;
    float viscosity = 1.5f;
    float influenceRadius = 0.013f;

    GLint UniformLocations[] = {
        glGetUniformLocation(computeShaderProgram.ID, "mass"),
        glGetUniformLocation(computeShaderProgram.ID, "stiffness"),
        glGetUniformLocation(computeShaderProgram.ID, "targetDensity"),
        glGetUniformLocation(computeShaderProgram.ID, "collisionDamping"),
        glGetUniformLocation(computeShaderProgram.ID, "gravity"),
        glGetUniformLocation(computeShaderProgram.ID, "deltaTime"),
        glGetUniformLocation(computeShaderProgram.ID, "viscosity"),
        glGetUniformLocation(computeShaderProgram.ID, "influenceRadius")
    };

    glUniform1f(UniformLocations[0], mass);
    glUniform1f(UniformLocations[1], stiffness);
    glUniform1f(UniformLocations[2], targetDensity);
    glUniform1f(UniformLocations[3], collisionDamping);
    glUniform1f(UniformLocations[4], gravity);
    glUniform1f(UniformLocations[5], fixedDeltaTime);
    glUniform1f(UniformLocations[6], viscosity);
    glUniform1f(UniformLocations[7], influenceRadius);

    GLint mouseInfoUniLoc = glGetUniformLocation(computeShaderProgram.ID, "mouseInfo");
    glUniform3f(mouseInfoUniLoc, mouseInfo.x, mouseInfo.y, mouseInfo.z);

    double lastTime = glfwGetTime();
    double thisTime;
    double deltaTime;
    double timer = 0;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 430");

    int particlesNumberGui = particlesNumber;
    float spawnPos[2] = {spawnRegion.position.x, spawnRegion.position.y};
    float spawnSize[2] = {spawnRegion.size.x, spawnRegion.size.y};

    while(!glfwWindowShouldClose(window)) {
        thisTime = glfwGetTime();
        deltaTime = thisTime - lastTime;
        lastTime = thisTime;

        if (thisTime - timer > 1) {
            std::cout << "\r\033[32mFrames per second: " << 1 / deltaTime << std::flush;
            timer = thisTime;
        }

        processInput(window);
        glUseProgram(computeShaderProgram.ID);
        glUniform3f(mouseInfoUniLoc, mouseInfo.x, mouseInfo.y, mouseInfo.z);

        for (int i = 0; i < 8; i++) {
            int clearValue = -1;
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, cellHeadSSBO);
            glClearBufferData(GL_SHADER_STORAGE_BUFFER, GL_R32I, GL_RED_INTEGER, GL_INT, &clearValue);

            glUseProgram(computeShaderProgram.ID);
            glUniform1i(phaseUniformLocation, 0);
            glDispatchCompute(particlesNumber / 128, 1, 1);

            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT); // Wait for everyone to have registered into the grid
        
            glUniform1i(phaseUniformLocation, 1);
            glDispatchCompute(particlesNumber / 128, 1, 1);

            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

            glUniform1i(phaseUniformLocation, 2);
            glDispatchCompute(particlesNumber / 128, 1, 1);

            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        }

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        shaderProgram.Activate();
        glBindVertexArray(VAO);
        glDrawElementsInstanced(GL_TRIANGLES, meshIndices.size(), GL_UNSIGNED_INT, 0, particlesNumber);

        ImGui::Begin("Physics variables");
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);

        ImGui::Text("Particles number will always be round to a multiple of 128");
        ImGui::SliderInt("particles number", &particlesNumberGui, 1, 200000);

        ImGui::SliderFloat("mass", &mass, 0.0f, 1.0f);
        ImGui::SliderFloat("stiffness", &stiffness, 0.0f, 500.0f);
        ImGui::SliderFloat("target density", &targetDensity, 0.0f, 5000.0f);
        ImGui::SliderFloat("viscosity", &viscosity, 0.0f, 5.0f);
        ImGui::SliderFloat("collision damping", &collisionDamping, 0.0f, 1.0f);
        ImGui::SliderFloat("gravity", &gravity, 0.0f, 50.0f);
        ImGui::SliderFloat("time step", &fixedDeltaTime, 0.0f, 0.01f, "%.5f");
        ImGui::SliderFloat("influence radius", &influenceRadius, 0.0f, 0.3f);
        ImGui::SliderFloat("radius", &radius, 0.0f, 0.2f);

        ImGui::SliderFloat2("spawn box position", spawnPos, -1.0f, 1.0f);
        ImGui::SliderFloat2("spawn box size", spawnSize, 0.0f, 2.0f);

        if (ImGui::Button("Launch")) {
            particlesNumberGui -= particlesNumberGui % 128;
            particlesNumber = particlesNumberGui;

            spawnRegion.position.x = spawnPos[0];
            spawnRegion.position.y = spawnPos[1];
            spawnRegion.size.x = spawnSize[0];
            spawnRegion.size.y = spawnSize[1];

            resetParticles(spawnRegion, particlesNumber, particles);

            // Resetting the buffers
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO);
            glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Particle) * particlesNumber, particles.data(), GL_DYNAMIC_COPY);
            
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleNextSSBO);
            glBufferData(GL_SHADER_STORAGE_BUFFER, particlesNumber * sizeof(int), nullptr, GL_DYNAMIC_COPY);

            glUseProgram(computeShaderProgram.ID);
            glUniform1f(UniformLocations[0], mass);
            glUniform1f(UniformLocations[1], stiffness);
            glUniform1f(UniformLocations[2], targetDensity);
            glUniform1f(UniformLocations[3], collisionDamping);
            glUniform1f(UniformLocations[4], gravity);
            glUniform1f(UniformLocations[5], fixedDeltaTime);
            glUniform1f(UniformLocations[6], viscosity);
            glUniform1f(UniformLocations[7], influenceRadius);
            glUniform1f(radiusUniLoc, radius);

            glUseProgram(shaderProgram.ID);
            glUniform1f(glGetUniformLocation(shaderProgram.ID, "radius"), radius);
        }

        ImGui::PopStyleVar();
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    std::cout << "\n";

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    shaderProgram.Delete();
    computeShaderProgram.Delete();

    glfwTerminate();
    return 0;
}
