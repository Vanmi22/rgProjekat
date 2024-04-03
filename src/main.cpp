#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <rg/Shader.h>
#include <rg/Camera.h>
#include <rg/Model.h>

#include <iostream>
#include <vector>
#include <time.h>

void frameBufferSizeCallBack(GLFWwindow *window, int width, int height);

void mouseCallBack(GLFWwindow *window, double xpos, double ypos);

void scrollCallBack(GLFWwindow *window, double xoffset, double yoffset);

void processInput(GLFWwindow *window);

void keyCallBack(GLFWwindow *window, int key, int scancode, int action, int mods);

//velicina prozora
const unsigned int SRC_WIDTH = 1280;
const unsigned int SRC_HEIGHT = 720;

//kamera
float lastX = SRC_WIDTH / 2.0f;
float lastY = SRC_HEIGHT / 2.0f;
bool firstMouse = true;

//promenljive za merenje vremena
float deltaTime = 0.0f;
float lastFrame = 0.0f;

//strukture za izvore svetla

//directional light
struct DirLight {
    glm::vec3 mDirection;
    glm::vec3 mAmbient;
    glm::vec3 mDiffuse;
    glm::vec3 mSpecular;
};

//spotlight
struct SpotLight {
    glm::vec3 mPosition;
    glm::vec3 mDirection;
    float mCutOff;
    float mOuterCutOff;
    glm::vec3 mAmbient;
    glm::vec3 mDiffuse;
    glm::vec3 mSpecular;
    float mConstant;
    float mLinear;
    float mQuadratic;
};

//point light
struct PointLight {
    glm::vec3 mPostion;
    glm::vec3 mAmbient;
    glm::vec3 mDiffuse;
    glm::vec3 mSpecular;
    float mConstant;
    float mLinear;
    float mQuadratic;
};

Camera camera;

DirLight dirLight{};

int main() {
    //glfw: inicijalizacija i konfiguracija
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_CORE_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    //glfw kreiranje prozora
    GLFWwindow  *window = glfwCreateWindow(SRC_WIDTH, SRC_HEIGHT, "rgProjekat", NULL, NULL);
    if (window == NULL) {
        std::cout << "Neuspesno kreiranje GLFW prozora" << "\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, frameBufferSizeCallBack);
    glfwSetCursorPosCallback(window, mouseCallBack);
    glfwSetScrollCallback(window, scrollCallBack);
    glfwSetKeyCallback(window, keyCallBack);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    //glad: ucitavanje svih OpenGL funkcija
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Neuspasna inicijalizacija GLAD-a" << "\n";
        return -1;
    }

    //stbi_set_flip_vertically_on_load(true);

    //konfiguracija OpenGL-a
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //kreiranje shader-a
    Shader ourShader("resources/shaders/vertex_shader.vs", "resources/shaders/direction_light.fs");

    //Shader ourShader2("resources/shaders/vertex_shader.vs", "resources/shaders/direction_light.fs");

    //ucitavanje modela
    Model ourModel("resources/objects/grass/10450_Rectangular_Grass_Patch_v1_iterations-2.obj");
    ourModel.SetShaderTextureNamePrefix("material.");

    Model ourModel2("resources/objects/tree/Tree.obj");
    ourModel2.SetShaderTextureNamePrefix("material1.");

    dirLight.mDirection = glm::vec3(-0.0, -0.0f, -1.0f);
    dirLight.mAmbient = glm::vec3(0.1f, 0.1f, 0.1f);
    dirLight.mDiffuse = glm::vec3(1.0f, 1.0f, 1.0f);
    dirLight.mSpecular = glm::vec3(1.0f, 1.0f, 1.0f);

    camera.m_position = glm::vec3(0.0f, 2.0f, 0.0f);

    std::vector<float> floats(100);

    for (unsigned int i = 0; i < 100; i++) {
        floats.at(i) = ((float) random()/2147483646) * 2 - 1;
    }

    //petlja za renderovanje
    while (!glfwWindowShouldClose(window)) {

        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        //ulazi
        processInput(window);

        //render
        glClearColor(0.3f, 0.7f, 0.8f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ourShader.use();
        ourShader.setVec3("directional_light.m_direction", dirLight.mDirection);
        ourShader.setVec3("directional_light.m_ambient", dirLight.mAmbient);
        ourShader.setVec3("directional_light.m_diffuse", dirLight.mDiffuse);
        ourShader.setVec3("directional_light.m_specular", dirLight.mSpecular);

        ourShader.setVec3("viewPosition", camera.m_position);

        ourShader.setFloat("material.m_shininess", 32.0f);


        glm::mat4 projection = glm::perspective(glm::radians(camera.m_zoom),
                                                (float) SRC_WIDTH / (float) SRC_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        glm::vec3 modelScale = glm::vec3(1.0f);

        for (unsigned int i = 0; i < 100; i++) {
            float f = floats.at(i) * 100;
            glm::mat4 model2 = glm::mat4(1.0f);
            glm::vec3 pos = glm::vec3(glm::sin(glm::radians((float) i * 2))*f, 0.0f, glm::cos(glm::radians((float) i * 2))*f);
            model2 = glm::translate(model2, pos);
            model2 = glm::scale(model2, modelScale);
            ourShader.setMat4("model", model2);
            ourModel2.Draw(ourShader);
        }

        /*
        glm::mat4 model = glm::mat4(1.0f);
        glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f);
        model = glm::translate(model, pos);
        model = glm::scale(model, modelScale);
        ourShader.setMat4("model", model);
        ourModel2.Draw(ourShader);*/

        modelScale = glm::vec3(0.5f);

        glm::mat4 model = glm::mat4(1.0f);
        glm::vec3 pos = glm::vec3(0.0f, -6.0f, 0.0f);
        model = glm::translate(model, pos);
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, modelScale);
        ourShader.setMat4("model", model);
        ourModel.Draw(ourShader);

        //glfw: zameni buffer-e i proveri ulaze (pritisnuti dugmici, pomeren mis)
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

void frameBufferSizeCallBack(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouseCallBack(GLFWwindow *window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scrollCallBack(GLFWwindow *window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(yoffset);
}

void keyCallBack(GLFWwindow *window, int key, int scancode, int action, int mods) {}