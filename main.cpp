#define GLM_ENABLE_EXPERIMENTAL
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "shader.h"

std::vector<float> sphereVertices;
std::vector<unsigned int> sphereIndices;

//camera variable
float deltaTime = 0.0f;
float lastFrame = 0.0f;

glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f,  3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f); 
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);

float cameraRadius = 3.0f;          
float orbitAngle = 0.0f;       
float orbitSpeed = 1.5f;    
float zoomSpeed = 1.0f;   

void generateUVSphere(float radius, int sectorCount, int stackCount, std::vector<float>& vertices) 
{
    vertices.clear();

    float x, y, z, xy;
    float sectorStep = 2 * M_PI / sectorCount;
    float stackStep = M_PI / stackCount;
    float sectorAngle, stackAngle;

    for(int i = 0; i <= stackCount; ++i) {
        stackAngle = M_PI / 2 - i * stackStep;        //from pi/2 to -pi/2
        xy = radius * cosf(stackAngle);             
        z = radius * sinf(stackAngle);             

        for(int j = 0; j <= sectorCount; ++j) {
            sectorAngle = j * sectorStep;           // from 0 to 2pi

            x = xy * cosf(sectorAngle);             
            y = xy * sinf(sectorAngle);             
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
        }
    }
}

void generateUVSphereIndices(int sectorCount, int stackCount, std::vector<unsigned int>& indices) 
{
    indices.clear();

    int k1, k2;
    for (int i = 0; i < stackCount; ++i) {
        k1 = i * (sectorCount + 1);    
        k2 = k1 + sectorCount + 1;      

        for (int j = 0; j < sectorCount; ++j, ++k1, ++k2) {

            if (i != 0) {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }

            if (i != (stackCount - 1)) {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0,0, width, height);
}

void processInput(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }

    const float cameraSpeed = 2.5f * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) 
    {
        // zoom in
        cameraRadius -= zoomSpeed * cameraSpeed;
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) 
    {
        //zoom out
        cameraRadius += zoomSpeed * cameraSpeed;
    }

    //clamp camera orbit radius
    if (cameraRadius < 0.5f) cameraRadius = 0.5f;
    if (cameraRadius > 20.0f) cameraRadius = 20.0f;

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) 
    {
        orbitAngle += orbitSpeed * deltaTime;
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) 
    {
        orbitAngle -= orbitSpeed * deltaTime;
    }
}

std::string loadShaderSource(const char* filePath) 
{
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Failed to open shader file: " << filePath << std::endl;
        exit(EXIT_FAILURE);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

int main()
{
    if (!glfwInit()) return -1;

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); 
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Tutorial", NULL, NULL);
    if ( window == NULL)
    {
        std::cout << "Failed to create window" <<  std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // Load OpenGL functions using GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // enable depth testing
    glEnable(GL_DEPTH_TEST);

    // set up sphere model
    float sphereRadius = 0.5f;
    int sectorCount = 36;  // longitude 
    int stackCount = 18;   // latitude 

    generateUVSphere(sphereRadius, sectorCount, stackCount, sphereVertices);
    generateUVSphereIndices(sectorCount, stackCount, sphereIndices);    

    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    //bind vao
    glBindVertexArray(VAO);

    //vertex data
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(float), &sphereVertices[0], GL_STATIC_DRAW);

    //element data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices.size() * sizeof(unsigned int), &sphereIndices[0], GL_STATIC_DRAW);
    
    //vertex attribute pointer
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    //safety unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    Shader shader("shaders/vertex_shader.glsl", "shaders/fragment_shader.glsl");

    glViewport(0, 0, 800, 600);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // main render loop
    while(!glfwWindowShouldClose(window))
    {
        //time
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Poll input events
        processInput(window);

        // Clear colour and depth buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float time = (float)glfwGetTime();

        //model matrix
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, time, glm::vec3(1.0f, 0.0f, 0.0f));

        //view - camera
        cameraPos.x = cameraRadius * sin(orbitAngle);
        cameraPos.z = cameraRadius * cos(orbitAngle);
        glm::mat4 view = glm::lookAt(cameraPos, glm::vec3(0.0f), cameraUp);

        //projection
        float aspectRatio = 800.0f / 600.0f;
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
        
        //model view projection
        glm::mat4 mvp = projection * view * model;
        
        //Use shaders
        shader.use();
        shader.setMat4("model", model);
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);

        //draw here
        glClearColor(0.1f, 0.4f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        //draw
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, (GLsizei)sphereIndices.size(), GL_UNSIGNED_INT, 0);

        //safety unbind
        glBindVertexArray(0);


        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
